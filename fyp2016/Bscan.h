#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "Log.h"
#include "Ascan.h"

#define MAX_LENGTH 800

class Bscan
{
private:
	std::vector<Ascan*> scans;
public:
	Bscan();
	~Bscan();

	void load(std::string filename);
	Ascan* element(int);
	int length();

	Ascan* produceNormal(int);
	void normalise(Ascan*);
	bool add(Ascan*);


	int Kernel(int dimension, int h, int d);
};

