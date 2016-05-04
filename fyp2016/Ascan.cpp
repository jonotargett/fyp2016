#include "Ascan.h"



Ascan::Ascan(int l)
{
	length = l;
	offset = 0;

	values = new int[length];
}


Ascan::~Ascan()
{
}


void Ascan::fill(int entry) {
	if (offset < length) {
		values[offset] = entry;
		offset++;
	}
}

int Ascan::getSize() {
	return length;
}

int Ascan::getIndex(int index) {
	if (index < 0)
		return 0;
	if (index > length)
		return 0;

	return values[index];
}

int Ascan::getPercent(float percent) {
	if (percent <= 0.0f)
		return values[0];
	if (percent >= 1.0f)
		return values[length - 1];

	int index = (int)percent * length;

	return values[index];
}