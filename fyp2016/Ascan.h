#pragma once
#include <cstdint>

class Ascan
{
private:
	int offset;
	const int length;
	const uint16_t *values;
public:
	Ascan(int, uint16_t*);
	~Ascan();

	int getSize();
	int getIndex(int);
	int getPercent(float);
};

