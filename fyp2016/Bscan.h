#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "Log.h"
#include "Ascan.h"

#define MAX_LENGTH 800

#define KERNEL_GAIN 32
#define VERTICAL_GAIN 1
#define HORIZONTAL_GAIN 0.4

enum AntType {
	ANT_CHANNEL1,
	ANT_CHANNEL2,
	ANT_CHANNEL_DIFF
};


class Bscan
{
private:
	std::vector<Ascan*> scans;
public:
	Bscan();
	~Bscan();

	void load(std::string filename);
	void loadPlainText(std::string filename);
	void loadRDR(std::string filename, AntType);
	Ascan* element(int);
	int length();

	Ascan* produceNormal(int);
	void normalise(Ascan*);
	bool add(Ascan*);


	int Kernel(int dimension, unsigned int h, unsigned int d);
};

