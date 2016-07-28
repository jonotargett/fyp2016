#include "Ascan.h"



Ascan::Ascan(unsigned int l, uint16_t* v) : length(l), values(v) {
	offset = 0;
	/*
	values = new int[length];

	for (int i = 0; i < length; i++) {
		values[i] = v[i];
	}
	*/
}


Ascan::~Ascan()
{
	delete values;
}



unsigned int Ascan::getSize() {
	return length;
}

uint16_t Ascan::getIndex(unsigned int index) {
	if (index > length)
		return 0;

	return values[index];
}

uint16_t Ascan::getPercent(float percent) {
	if (percent <= 0.0f)
		return values[0];
	if (percent >= 1.0f)
		return values[length - 1];

	unsigned int index = (unsigned int)percent * length;

	return values[index];
}