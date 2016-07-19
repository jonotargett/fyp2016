#include "GPR.h"
#include "Log.h"

GPR::GPR() {
	// do nothing

	buffer = new unsigned int[8192];

	status = 0x0;

	ids = new id_struct();
	ids->antenna_id = 0xDEADBEEF;
	ids->low_battery = 0x1;
	ids->pga_id[0] = 'a';
	ids->pgm_id[0] = 'a';		// just set to anything non-default

	return;
}

GPR::~GPR() {
	//nothing to clear

	delete buffer;

	return;
}

bool GPR::is_bit_set(unsigned int val, unsigned int bit) {
	return !!((val) & (1 << (bit)));
}


bool GPR::initialise() {

	// GENERATE PARAMETERS ---------------------------//

	//BUF0
	bool fullParam = true;
	char DA_Delay = 0;						// range [0, 255]

	//BUF1
	short timeBase = 0;						// range [0, 4095]

	//BUF2
	char cable_Delay = 0;					// range [0, 7]
	GPR_FRAMERATE framerate = GPR_FRAMERATE_254Hz;

	//BUF3
	char analogGain = 15;					// range [0, 127]

	//BUF4
	char singleAntennaGain = 1;				// range [0, 3]
	char differentialAntennaGain = 0;		// range [0, 3]
	GPR_SPI_UPDATING spi = GPR_SPI_UPDATING_ENABLE;
	GPR_PRF prf = GPR_PRF_65kHz;
	GPR_AD_AVERAGING ad_av = GPR_AD_AVERAGING_DISABLE;
	GPR_AD_CALIBRATION ad_cal = GPR_AD_CALIBRATION_DISABLE;

	// TODO: flesh out this step

	unsigned int buf[5];
	buf[0] = 0x80000000;
	buf[1] = 0x00000000;
	buf[2] = 0x00000000;
	buf[3] = 0x0000000F;
	buf[4] = 0x00000000;

	//------------------------------------------------//

	int ret = 0x0;
									// connect to the GPR and attempt to set scanning parameters
	while (ret != 0x00) {
		ret = set_parameters(buf);

		if (ret == 0x01) {
			Log::e << "device is offline. Code: " << ret << endl;
			return false;
		}
		else if (ret == 0x10) {
			Log::e << "device write queue is full. Code: " << ret << endl;
			return false;
		}
		else {
			Log::i << "appears to be well. code: " << ret << endl;
		}
	}



	// synchronise data stream
	Log::i << "synchronising data stream..." << endl;
	status = read_data(buffer, -1, ids);

	// read status data    
	Log::i << "reading status data..." << endl;


	status = 1;

	while (status != 0x0) {
		status = read_data(buffer, 0, ids);

		if (status == 0x0) {
			Log::i << "all okay. Code: " << status << endl;

			int antenna_id = ids->antenna_id;
			int low_batt = ids->low_battery;
			char* fpga_id = new char[31];
			char* firmware_ver = new char[31];

			strncpy(fpga_id, ids->pga_id, 30);
			fpga_id[30] = '\0';
			strncpy(firmware_ver, ids->pgm_id, 30);
			firmware_ver[30] = '\0';

			Log::i << "\tantenna id: " << antenna_id << endl;
			Log::i << "\tlow battery? " << low_batt << endl;
			Log::i << "\tFPGA ID: " << fpga_id << "|" << endl;
			Log::i << "\tfirmware version: " << firmware_ver << endl;
		}
		else {
			Log::e << "error. Code: " << status << endl;
			return false;
		}
	}


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