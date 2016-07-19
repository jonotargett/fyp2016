#pragma once
#include "csirousb.h"

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
