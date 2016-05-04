#pragma once
class Ascan
{
private:
	int offset;
	const int length;
	const int *values;
public:
	Ascan(int, int*);
	~Ascan();

	int getSize();
	int getIndex(int);
	int getPercent(float);
};

