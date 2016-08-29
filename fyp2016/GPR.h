#pragma once
#include <iomanip>
#include <bitset>
#include <fstream>
#include <thread>

#include "csirousb.h"
#include "Bscan.h"


#define MAX_ATTEMPTS 10
#define CHANNEL_STRIDE 512
#define DIGITAL_GAIN 512
#define SIGNED_OFFSET 32768

enum GPR_CHANNEL {
	GPR_DIFFERENTIAL,
	GPR_CHANNEL_1,
	GPR_CHANNEL_2
};

enum GPR_PARAM_UPDATE {
	GPR_PARAM_UPDATE_CHANGES_ONLY,
	GPR_PARAM_UPDATE_FLUSH_ALL
};

enum GPR_PRF {
	GPR_PRF_65kHz,
	GPR_PRF_130kHz,
	GPR_PRF_512kHz,
	GPR_PRF_1MHz
};

enum GPR_FRAMERATE {
	GPR_FRAMERATE_254Hz,
	GPR_FRAMERATE_127Hz
};

enum GPR_SPI_UPDATING {
	GPR_SPI_UPDATING_ENABLE,
	GPR_SPI_UPDATING_DISABLE
};

enum GPR_AD_AVERAGING {
	GPR_AD_AVERAGING_DISABLE,
	GPR_AD_AVERAGING_ENABLE
};

enum GPR_AD_CALIBRATION {
	GPR_AD_CALIBRATION_DISABLE,
	GPR_AD_CALIBRATION_ENABLE
};

class GPR {

public:
	GPR();
	~GPR();

	bool shouldExit;

	bool initialise();
	bool flushParams();

	bool constThread();

	
	bool checkStatus(bool);
	Bscan* getBscan(GPR_CHANNEL);

private:
	bool getData(bool = false);

	int range;
	bool properly_initialised;
	std::thread* updater;

	bool green_button;
	bool red_button;
	bool yellow_button;
	bool blue_button;

	Bscan* differential;
	Bscan* channel1;
	Bscan* channel2;

	GPR_PARAM_UPDATE updateMode;
	GPR_SPI_UPDATING serialUpdating;
	GPR_FRAMERATE framerate;
	GPR_PRF prf;
	GPR_AD_AVERAGING adAveraging;
	GPR_AD_CALIBRATION adCalibration;

	unsigned int daDelay;					// range [0, 255]
	unsigned int timeBase;					// range [0, 4095]
	unsigned int cableDelay;				// range [0, 7]
	unsigned int analogGain;				// range [0, 127]
	unsigned int singleAntennaGain;			// range [0, 3]
	unsigned int differentialAntennaGain;	// range [0, 3]
	
	unsigned int* params;
	unsigned int* dataBuffer;
	unsigned int status;
	unsigned int samples;
	id_struct* ids;

	bool is_bit_set(unsigned int, unsigned int);

	void setFlushMode(GPR_PARAM_UPDATE);
	void enableSPIUpdate(GPR_SPI_UPDATING);
	void setGPR_PRF(GPR_PRF);
	void enableADAveraging(GPR_AD_AVERAGING);
	void enableADAutoCalibration(GPR_AD_CALIBRATION);
	bool setDADelay(unsigned int);
	bool setCableDelay(unsigned int);
	bool setTimeBase(unsigned int);
	bool setAnalogGain(unsigned int);
	bool setSingleAntennaGain(unsigned int);
	bool setDifferentialAntennaGain(unsigned int);

	bool processParams();
	bool processStatusCode(bool verbose = false);
};
