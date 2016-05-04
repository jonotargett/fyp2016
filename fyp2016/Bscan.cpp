#include "Bscan.h"

Bscan::Bscan()
{
}


Bscan::~Bscan()
{
}


void Bscan::load(std::string filename) {

	std::ifstream file;
	char* memblock;
	int offset = 0;
	int size = 0;



	file.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	//file.open("C:/HaxLogs.txt", std::ios::in | std::ios::binary | std::ios::ate);
	//01011329.DAT
	//file.open("C:/Users/Jono/Documents/Visual Studio 2015/Projects/fyp2016/Debug/01011348.DAT", std::ios::in | std::ios::binary | std::ios::ate);

	if (!file.is_open()) {
		char* buf = new char[256];
		strerror_s(buf, 256, errno);
		std::cout << "Error: " << buf;
		return;
	}


	size = file.tellg();		//reports size in bytes?
	memblock = new char[size];		// allocate enough space to store the file in RAM

	file.seekg(0, std::ios::beg);
	file.read(memblock, size);

	file.close();


	std::cout << size << std::endl;


	int ASCAN_STRIDE = 512;

	while (offset < size) {

		Ascan* scan = new Ascan(ASCAN_STRIDE);
		int count = 0;
		
		while ((count < (ASCAN_STRIDE*2)) && offset < size) {

			int e = (uint8_t)memblock[offset+1]*256 + (uint8_t)memblock[offset];

			scan->fill(e);

			count += 2;
			offset += 2;
		}

		scans.push_back(scan);
	}
	
	delete memblock;
	memblock = NULL;
	
}


int Bscan::length() {
	return scans.size();
}


Ascan* Bscan::element(int index) {
	if (index < length()) {
		return scans.at(index);
	}

	return NULL;
}


Ascan* Bscan::produceNormal(int firstXscans) {

	if (scans.size() < firstXscans || scans.size() < 1) {
		return NULL;
	}

	Ascan* ret = new Ascan(scans.at(0)->getSize());

	for (int i = 0; i < ret->getSize(); i++) {

		double sum = 0;

		for (int j = 0; j < firstXscans; j++) {
			sum += scans.at(j)->getIndex(i);
		}

		sum /= firstXscans;

		ret->fill((int)sum);
	}

	return ret;
}

void Bscan::normalise(Ascan* normal) {

	std::vector<Ascan*>::iterator it;
	

	for (int i = 0; i < scans.size(); i++) {
		Ascan* newScan = new Ascan(scans.at(i)->getSize());

		for (int j = 0; j < normal->getSize(); j++) {
			int dif = scans.at(i)->getIndex(j) - normal->getIndex(j);

			newScan->fill(127*256 + dif);
		}
		
		Ascan* old = scans.at(i);
		it = scans.begin();
		scans.erase(it + i);
		it = scans.begin();
		scans.insert(it + i, newScan);
		delete old;
		old = NULL;		
		
	}
}