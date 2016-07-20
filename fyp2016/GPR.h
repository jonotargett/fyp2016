#pragma once
#include "csirousb.h"


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

class GPR
{
private:
	unsigned int* buffer;
	unsigned int status;
	id_struct* ids;

	bool is_bit_set(unsigned int, unsigned int);
public:
	GPR();
	~GPR();

	bool initialise();
	bool getData();
};
