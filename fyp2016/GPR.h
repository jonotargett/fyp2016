#pragma once
#include "csirousb.h"

#define MAX_ATTEMPTS 10

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

	bool initialise();
	bool flushParams();

	bool getData();
	bool checkStatus(bool);

private:
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
	bool processStatusCode();
};
