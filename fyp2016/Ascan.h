#pragma once
#include <cstdint>

class Ascan
{
private:
	unsigned int offset;
	const unsigned int length;
	const uint16_t *values;
public:
	Ascan(unsigned int, uint16_t*);
	~Ascan();

	unsigned int getSize();
	uint16_t getIndex(unsigned int);
	uint16_t getPercent(float);
};

