#include "GPR.h"
#include "Log.h"

GPR::GPR() {
	//BUF0
	GPR_PARAM_UPDATE updateMode = GPR_PARAM_UPDATE_FLUSH_ALL;
	unsigned int DA_Delay = 16;					// range [0, 255]

	//BUF1
	unsigned int timeBase = 0;					// range [0, 4095]

	//BUF2
	unsigned int cableDelay = 1;					// range [0, 7]
	GPR_FRAMERATE framerate = GPR_FRAMERATE_127Hz;

	//BUF3
	unsigned int analogGain = 15;					// range [0, 127]

	//BUF4
	unsigned int singleAntennaGain = 1;				// range [0, 3]
	unsigned int differentialAntennaGain = 0;		// range [0, 3]
	GPR_SPI_UPDATING spi = GPR_SPI_UPDATING_ENABLE;
	GPR_PRF prf = GPR_PRF_65kHz;
	GPR_AD_AVERAGING ad_av = GPR_AD_AVERAGING_ENABLE;
	GPR_AD_CALIBRATION ad_cal = GPR_AD_CALIBRATION_DISABLE;

	//----------------------------------------------------------------------

	params = new unsigned int[5];
	buffer = new unsigned int[8192];
	ids = new id_struct();
	status = 0x0;

	
	ids->antenna_id = 0xDEADBEEF;
	ids->low_battery = 0x1;
	ids->pga_id[0] = 'a';
	ids->pgm_id[0] = 'a';		// just set to anything non-default

	return;
}

GPR::~GPR() {
	//nothing to clear

	delete buffer;
	delete params;
	delete ids;

	return;
}


bool GPR::is_bit_set(unsigned int val, unsigned int bit) {
	return !!((val) & (1 << (bit)));
}




void GPR::setFlushMode(GPR_PARAM_UPDATE p) {
	updateMode = p;
}
void GPR::enableSPIUpdate(GPR_SPI_UPDATING p) {
	spi = p;
}
void GPR::setGPR_PRF(GPR_PRF p) {
	prf = p;
}
void GPR::enableADAveraging(GPR_AD_AVERAGING p) {
	ad_av = p;
}
void GPR::enableADAutoCalibration(GPR_AD_CALIBRATION p) {
	ad_cal = p;
}
bool GPR::setDADelay(unsigned int p) {
	if (p > 255) return false;
	DA_Delay = p;
	return true;
}
bool GPR::setCableDelay(unsigned int p) {
	if (p > 7) return false;
	cableDelay = p;
	return true;
}
bool GPR::setTimeBase(unsigned int p) {
	if (p > 4095) return false;
	timeBase = p;
	return true;
}
bool GPR::setAnalogGain(unsigned int p) {
	if (p > 127) return false;
	analogGain = p;
	return true;
}
bool GPR::setSingleAntennaGain(unsigned int p) {
	if (p > 3) return false;
	singleAntennaGain = p;
	return false;
}
bool GPR::setDifferentialAntennaGain(unsigned int p) {
	if (p > 3) return false;
	differentialAntennaGain = p;
	return true;
}

bool GPR::processParams() {

	// GENERATE PARAMETERS ---------------------------//
	/*
	Place all of the data collected into the correct bit format,
	in a buffer of 5 unsigned ints. The correct bit format is explained
	in the csirousb.h file. This format was created by the CSIRO.
	*/
	
	unsigned int mask = 0x0;

	// BUFFER 0
	params[0] = 0x0;
	if (updateMode == GPR_PARAM_UPDATE_FLUSH_ALL) params[0] = 0x1;
	params[0] = params[0] << 31;
	mask = 0x000000FF;
	DA_Delay = DA_Delay & mask;
	params[0] = params[0] + DA_Delay;

	// BUFFER 1
	mask = 0x00000FFF;
	timeBase = timeBase & mask;
	params[1] = timeBase;

	// BUFFER 2
	params[2] = 0x0;
	mask = 0x00000007;
	cableDelay = cableDelay & mask;
	params[2] += cableDelay;
	params[2] = params[2] << 1;
	if (framerate == GPR_FRAMERATE_127Hz) params[2] += 0x1;
	params[2] = params[2] << 4;

	// BUFFER 3
	mask = 0x0000007F;
	analogGain = analogGain & mask;
	params[3] = analogGain;

	// BUFFER 4
	params[4] = 0x0;
	mask = 0x00000003;
	singleAntennaGain = singleAntennaGain & mask;
	differentialAntennaGain = differentialAntennaGain & mask;

	params[4] += singleAntennaGain;
	params[4] = params[4] << 2;
	params[4] += differentialAntennaGain;
	params[4] = params[4] << 1;
	if (spi == GPR_SPI_UPDATING_DISABLE) params[4] += 0x1;
	params[4] = params[4] << 2;
	switch (prf) {
	default:
	case GPR_PRF_65kHz:		params[4] += 0x00;	break;
	case GPR_PRF_130kHz:	params[4] += 0x01;	break;
	case GPR_PRF_512kHz:	params[4] += 0x10;	break;
	case GPR_PRF_1MHz:		params[4] += 0x11;	break;
	}
	params[4] = params[4] << 2;
	if (ad_av == GPR_AD_AVERAGING_ENABLE) params[4] += 0x01;
	params[4] = params[4] << 2;
	if (ad_cal == GPR_AD_CALIBRATION_ENABLE) params[4] += 0x01;
	params[4] = params[4] << 11;
	
	return true;
}

bool GPR::initialise() {

	int attempts = 0;
	int maxattempts = 3;
	int ret = 0xF;			//anything non-zero
	bool success = false;


	//generate the configuration parameters that will
	// be sent to initialise the hardware
	processParams();


	// connect to the GPR and attempt to set scanning parameters
	// attempt a connection up to MAXATTEMPTS many times before failing and
	// returning false.
	while (ret != 0x00 && attempts < maxattempts) {
		ret = set_parameters(params);

		if (ret == 0x01) {
			Log::e << "GPR device is offline. Code: " << ret << endl;
			attempts++;
		}
		else if (ret == 0x10) {
			Log::e << "GPR device write queue is full. Code: " << ret << endl;
			attempts++;
		}
		else {
			Log::i << "GPR appears to be connected. code: " << ret << endl;
			success = true;
		}
	}

	if (!success)
		return false;

	success = false;
	attempts = 0;

	// synchronise data stream
	Log::i << "synchronising data stream..." << endl;
	status = read_data(buffer, -1, ids);

	// read status data    
	Log::i << "reading status data..." << endl;


	status = 1;	//anything non-zero

	while (status != 0x0 && attempts < maxattempts) {
		status = read_data(buffer, 0, ids);

		if (status == 0x0) {
			Log::i << "GPR status all okay. Code: " << status << endl;

			int antenna_id = ids->antenna_id;
			int low_batt = ids->low_battery;
			char* fpga_id = new char[31];
			char* firmware_ver = new char[31];

			strncpy(fpga_id, ids->pga_id, 30);
			fpga_id[30] = '\0';
			strncpy(firmware_ver, ids->pgm_id, 30);
			firmware_ver[30] = '\0';

			Log::i << "\tAntenna ID:       " << antenna_id << endl;
			Log::i << "\tLow battery?      " << low_batt << endl;
			Log::i << "\tFPGA ID:          " << fpga_id << endl;
			Log::i << "\tfirmware version: " << firmware_ver << endl;
		}
		else {
			Log::e << "GPR status error. Code: " << status << endl;
			attempts++;
		}
	}

	if (!success)
		return false;


	return true;
}


bool GPR::getData() {
	Log::i << "wait for data..." << endl;

	unsigned int mask = ((1 << 8) - 1) << 16;
	unsigned int samples = 0;
	int error_chances = 2000;

	while (samples == 0 && error_chances > 0) {

		status = read_data(buffer, 1, ids);
		samples = (status & mask) >> 16;
		bool hasErrored = false;
		if (samples == 0) {
			continue;
		}
		Log::i << "---" << endl;
		printf("%x\n", status);
		printf("%x\n", samples);
		Log::i << "---" << endl;
		Log::i << "SAMPLES: " << samples << endl;




		if (is_bit_set(status, 31)) {
			Log::d << "not enough data in DLL buffer" << endl;
			hasErrored = true;
		}
		if (is_bit_set(status, 30)) {
			Log::d << "synch operation succeeded (why is this an error?)" << endl;
			hasErrored = true;
		}
		if (is_bit_set(status, 11)) {
			Log::d << "micro buffer overflow" << endl;
			hasErrored = true;
		}
		if (is_bit_set(status, 0)) {
			Log::d << "device offline" << endl;
			hasErrored = true;
		}
		if (is_bit_set(status, 1)) {
			Log::d << "radar is off" << endl;
			hasErrored = true;
		}
		if (is_bit_set(status, 9)) {
			Log::d << "dll buffer got full" << endl;
			hasErrored = true;
		}
		if (is_bit_set(status, 8)) {
			Log::d << "invalid buffer size" << endl;
			hasErrored = true;
		}
		if (is_bit_set(status, 10)) {
			Log::d << "bad sequence number" << endl;
			hasErrored = true;
		}


		if (hasErrored) {
			samples = 0;
			Log::d << "Error. code: " << status << endl;
		}
		else {

			// read some data from the buffer
			Log::i << "retrieve data..." << endl;

			Log::i << "\t\t RETRIEVED DATA" << endl;
			status = read_data(buffer, 64 * samples, ids);
			for (unsigned int i = 0; i<64 * samples; i++) {
				Log::i << buffer[i] << " | ";
			}
			Log::i << endl;
			samples = 0;
		}


	}

	return true;
}