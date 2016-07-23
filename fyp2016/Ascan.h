#pragma once
#include <cstdint>

class Ascan
{
private:
	int offset;
	const int length;
	const int16_t *values;
public:
	Ascan(int, int16_t*);
	~Ascan();

	int getSize();
	int getIndex(int);
	int getPercent(float);
};

