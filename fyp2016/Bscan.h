#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "Ascan.h"

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
};

