#pragma once
class Ascan
{
private:
	int offset;
	int length;
	int* values;
public:
	Ascan(int);
	~Ascan();

	void fill(int);
	int getSize();
	int getIndex(int);
	int getPercent(float);
};

