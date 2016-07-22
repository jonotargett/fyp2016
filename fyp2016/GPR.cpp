#include "GPR.h"
#include "Log.h"

GPR::GPR() {
	params = new unsigned int[5];
	dataBuffer = new unsigned int[8192];
	ids = new id_struct();
	status = 0x0;

	// set default GPR configuration parameters
	updateMode = GPR_PARAM_UPDATE_FLUSH_ALL;
	serialUpdating = GPR_SPI_UPDATING_ENABLE;
	framerate = GPR_FRAMERATE_127Hz;
	prf = GPR_PRF_65kHz;
	adAveraging = GPR_AD_AVERAGING_ENABLE;
	adCalibration = GPR_AD_CALIBRATION_DISABLE;

	daDelay = 0;					// range [0, 255]
	timeBase = 0;					// range [0, 4095]
	cableDelay = 1;					// range [0, 7]
	analogGain = 15;					// range [0, 127]
	singleAntennaGain = 1;				// range [0, 3]
	differentialAntennaGain = 0;		// range [0, 3]
	
	green_button = false;
	red_button = false;
	yellow_button = false;
	blue_button = false;

	// initialise ID values to something non-default
	ids->antenna_id = 0xDEADBEEF;
	ids->low_battery = 0x1;
	ids->pga_id[0] = 'a';
	ids->pgm_id[0] = 'a';

	return;
}

GPR::~GPR() {

	delete dataBuffer;
	delete params;
	delete ids;

	return;
}


/*
Checks the value of a certain bit in an integer value.
returns true if bit is set, false if otherwise
*/
bool GPR::is_bit_set(unsigned int val, unsigned int bit) {
	return !!((val) & (1 << (bit)));
}



void GPR::setFlushMode(GPR_PARAM_UPDATE p) {
	updateMode = p;
}
void GPR::enableSPIUpdate(GPR_SPI_UPDATING p) {
	serialUpdating = p;
}
void GPR::setGPR_PRF(GPR_PRF p) {
	prf = p;
}
void GPR::enableADAveraging(GPR_AD_AVERAGING p) {
	adAveraging = p;
}
void GPR::enableADAutoCalibration(GPR_AD_CALIBRATION p) {
	adCalibration = p;
}
bool GPR::setDADelay(unsigned int p) {
	if (p > 255) return false;
	daDelay = p;
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

/*
Place all of the parameter data into the correct bit format,
in a buffer of 5 unsigned ints. The correct bit format is explained
in the csirousb.h file. This format was created by the CSIRO.
*/
bool GPR::processParams() {

	unsigned int mask = 0x0;

	// BUFFER 0
	params[0] = 0x0;
	if (updateMode == GPR_PARAM_UPDATE_FLUSH_ALL) params[0] = 0x1;
	params[0] = params[0] << 31;
	mask = 0x000000FF;
	daDelay = daDelay & mask;
	params[0] = params[0] + daDelay;

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
	if (serialUpdating == GPR_SPI_UPDATING_DISABLE) params[4] += 0x1;
	params[4] = params[4] << 2;
	switch (prf) {
	default:
	case GPR_PRF_65kHz:		params[4] += 0x00;	break;
	case GPR_PRF_130kHz:	params[4] += 0x01;	break;
	case GPR_PRF_512kHz:	params[4] += 0x10;	break;
	case GPR_PRF_1MHz:		params[4] += 0x11;	break;
	}
	params[4] = params[4] << 2;
	if (adAveraging == GPR_AD_AVERAGING_ENABLE) params[4] += 0x01;
	params[4] = params[4] << 2;
	if (adCalibration == GPR_AD_CALIBRATION_ENABLE) params[4] += 0x01;
	params[4] = params[4] << 11;
	
	return true;
}

/*
This is the bit of code that actually sends the configuration data down
to the GPR hardware. It sends a snapshot of all of the parameter settings
at the current point in time and updates the GPR with that. Setting each
individual parameter will have no effect on the GPR until this is called.
*/
bool GPR::flushParams() {
	
	int attempts = 0;
	int ret = 0xF;			//anything non-zero

	processParams();

	// connect to the GPR and attempt to set scanning parameters
	// attempt a connection up to MAXATTEMPTS many times before failing and
	// returning false.
	while (ret != 0x00 && attempts < MAX_ATTEMPTS) {
		ret = set_parameters(params);

		if (ret == 0x01) {
			Log::e << "GPR device is offline. Code: " << std::hex << ret << std::dec << endl;
			attempts++;
		}
		else if (ret == 0x10) {
			Log::e << "GPR device write queue is full. Code: " << std::hex << ret << std::dec << endl;
			attempts++;
		}
		else {
			Log::i << "Configuration updated. code: " << std::hex << ret << std::dec << endl;
			return true;
		}
	}

	return false;
}

/*
Read the status code (member variable) and returns whether an error has occured.
True means success, false means failure.
Does not recheck the status of the hardware to ensure validity of the status code.
*/
bool GPR::processStatusCode() {

	unsigned int mask = ((1 << 8) - 1) << 16;
	samples = (status & mask) >> 16;

	bool hasErrored = false;

	if (is_bit_set(status, 31)) {
		Log::d << "\tnot enough data in DLL buffer" << endl;
		hasErrored = true;
	}
	if (is_bit_set(status, 30)) {
		Log::d << "\tsynch operation succeeded (why is this an error?)" << endl;
		hasErrored = true;
	}
	if (is_bit_set(status, 11)) {
		Log::d << "\tmicro buffer overflow" << endl;
		hasErrored = true;
	}
	if (is_bit_set(status, 0)) {
		Log::d << "\tdevice offline" << endl;
		hasErrored = true;
	}
	if (is_bit_set(status, 1)) {
		Log::d << "\tradar is off" << endl;
		hasErrored = true;
	}
	if (is_bit_set(status, 9)) {
		Log::d << "\tdll buffer got full" << endl;
		hasErrored = true;
	}
	if (is_bit_set(status, 8)) {
		Log::d << "\tinvalid buffer size" << endl;
		hasErrored = true;
	}
	if (is_bit_set(status, 10)) {
		Log::d << "\tbad sequence number" << endl;
		hasErrored = true;
	}

	if (hasErrored) {
		Log::d << "Error code: " << std::hex << status << std::dec << endl;
	}

	return !hasErrored;
}

/*
checks the status of the device by reading the status data from the hardware.
returns true if status is okay, false if otherwise. if verbose is set, will 
dump the data contained in the IDs struct to the informative log.
*/
bool GPR::checkStatus(bool verbose) {
	bool success = false;
	unsigned int attempts = 0;

	while (!success && attempts < MAX_ATTEMPTS) {

		status = read_data(dataBuffer, 0, ids);
		success = processStatusCode();

		if (success) {
			if (verbose) {
				Log::i << "GPR status all okay. Code: " << std::hex << status << std::dec << endl;

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
		}
		else {
			Log::e << "GPR status error. Code: " << std::hex << status << std::dec << endl;
			attempts++;
		}
	}

	return success;
}

/*
Must be run prior to any use of the GPR hardware. Performs first-run
connection and initialisation of the hardware, synchronises the data stream,
and checks the operation status of the device. returns true if all processes
completed successfully, false otherwise.
*/
bool GPR::initialise() {

	int attempts = 0;
	bool success = false;

	//generate the configuration parameters that will
	// be sent to initialise the hardware
	Log::i << "Initialising hardware parameters..." << endl;
	setFlushMode(GPR_PARAM_UPDATE_FLUSH_ALL);

	success = flushParams();
	if (!success)
		return false;

	setFlushMode(GPR_PARAM_UPDATE_CHANGES_ONLY);

	// synchronise data stream
	// not sure if this is necessary or if it even does anything.
	// but it was mentioned in the supplied header so ill do it regardless
	Log::i << "Synchronising data stream..." << endl;
	success = false;
	while (!success) {
		status = read_data(dataBuffer, -1, ids);
		success = processStatusCode();
	}

	// read status data
	Log::i << "Checking synchronisation and reading status data..." << endl;
	success = checkStatus(true);
	
	if (!success)
		return false;

	return true;
}


/*
TODO(Jono) : this is incomplete
*/
bool GPR::getData() {
	Log::i << "Wait for data..." << endl;
	bool success = false;

	// this checks whether the GPR is ready, and calculates the number of 
	// samples available to be read from the device. the number of samples
	// is calculated everytime processStatusCode is called, which is called
	// automatically by check-status.
	success = checkStatus(false);
		
	if (!success) {
		samples = 0;
		Log::e << "Cannot retreive data. Error code: " << std::hex << status << std::dec << endl;
		return false;
	}
		
	if (samples == 0) {
		Log::i << "No samples available" << endl;
		return false;
	}
	
	// read some data from the buffer
	Log::i << "Attemptint to retrieve data..." << endl;
	Log::i << "Samples: " << samples << endl;


	status = read_data(dataBuffer, 64 * samples, ids);

	unsigned int valuesMask = 0x0000FFFF;
	unsigned int switchesMask = 0x000F0000;
	unsigned int positionMask = 0x00300000;
	unsigned int antennasMask = 0x00C00000;
	unsigned int blueSwitchMask = 0x1;
	unsigned int greenSwitchMask = 0x2;
	unsigned int redSwitchMask = 0x4;
	unsigned int yellowSwitchMask = 0x8;
	unsigned int value, switches, position, antennas;


	std::ofstream myfile;
	myfile.open("output.csv");
	//write headers
	myfile << "ID, Antenna, Switches, Position, Value\n";


	for (unsigned int i = 0; i<64 * samples; i++) {

		//dataBuffer[i] = 0xFFFFFFFF;

		value = dataBuffer[i] & valuesMask;
		switches = (dataBuffer[i] & switchesMask) >> 16;
		position = (dataBuffer[i] & positionMask) >> 20;
		antennas = (dataBuffer[i] & antennasMask) >> 22;

		
		if ((switches & 0b0001) == 0b0000) {
			//Log::d << "blue button pressed" << endl;
			blue_button = true;
		} else {
			blue_button = false;
		}

		if ((switches & 0b0010) == 0b0000) {
			//Log::d << "green button pressed" << endl;
			green_button = true;
		} else {
			green_button = false;
		}
		
		if ((switches & 0b0100) == 0b0000) {
			//Log::d << "red button pressed" << endl;
			red_button = true;
		}
		else {
			red_button = false;
		}

		if ((switches & 0b1000) == 0b0000) {
			//Log::d << "yellow button pressed" << endl;
			yellow_button = true;
		}
		else {
			yellow_button = false;
		}



		//Log::i << "Value:\t" << std::hex << value << std::dec << endl;
		//Log::i << "Switches:\t" << std::bitset<4>(switches) << endl;
		//Log::i << "Position:\t" << std::bitset<2>(position) << endl;
		//Log::i << "Antenna:\t" << std::bitset<2>(antennas) << endl;

		//Log::i << std::bitset<2>(antennas) << "-" << value << " | ";

		myfile << i << ", " << std::bitset<2>(antennas) 
			<< ", " << std::bitset<4>(switches) 
			<< ", " << std::bitset<2>(position) 
			<< ", " << std::bitset<16>(value) << "\n";

	}
	Log::i << endl;

	
	myfile.close();

	return success;
}

/*
data word bits (d31,d30... d0)

d15-d0	- 16 bit A/D converter value; two's compliment
d23-d22	-	Antenna code(11 - differential ;10/01 - antenna1 / antenna2)
d21-d20	- Position sensor data
d19-d16	- switch data ( the four switches on the antenna/base unit)
*/