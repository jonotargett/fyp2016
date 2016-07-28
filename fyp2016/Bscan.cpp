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


	if (!file.is_open()) {
		char* buf = new char[256];
		strerror_s(buf, 256, errno);
		Log::e << "File Error: " << buf;
		return;
	}


	size = (int)file.tellg();		//reports size in bytes?
	memblock = new char[size];		// allocate enough space to store the file in RAM

	file.seekg(0, std::ios::beg);
	file.read(memblock, size);

	file.close();


	Log::d << "Filesize: " << size << std::endl;
	

	int ASCAN_STRIDE = 512;

	while (offset < size) {

		uint16_t* v = new uint16_t[ASCAN_STRIDE];
		int count = 0;
		
		while ((count < ASCAN_STRIDE) && offset < size) {

			//TODO(Jono) check whether this is meant to be unsigned or 2s complement
			uint16_t e = (uint8_t)memblock[offset+1]*256 + (uint8_t)memblock[offset];

			v[count] = e;

			count += 1;
			offset += 2;
		}

		Ascan* scan = new Ascan(ASCAN_STRIDE, v);
		scans.push_back(scan);
	}

	Log::d << scans.size() << " scans @ " << scans.at(0)->getSize() << std::endl;
	
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

// adds an Ascan to this Bscan. rejects
// and returns false if the Ascan is not the same length as the rest of the Bscan
bool Bscan::add(Ascan* a) {
	if (this->scans.size() == 0) {
		scans.push_back(a);
		return true;
	}

	if (a->getSize() == scans.at(0)->getSize()) {
		scans.push_back(a);

		if (scans.size() > MAX_LENGTH) {
			scans.erase(scans.begin(), scans.begin() + (scans.size() - MAX_LENGTH));
		}

		return true;
	}

	return false;
}

Ascan* Bscan::produceNormal(int firstXscans) {

	if ((int)scans.size() < firstXscans || scans.size() < 1) {
		return NULL;
	}


	int size = scans.at(0)->getSize();
	uint16_t* v = new uint16_t[size];
	

	for (int i = 0; i < size; i++) {

		double sum = 0;

		for (int j = 0; j < firstXscans; j++) {
			sum += scans.at(j)->getIndex(i);
		}

		sum /= firstXscans;

		v[i] = (int)sum;
	}

	Ascan* ret = new Ascan(size, v);
	return ret;
}

void Bscan::normalise(Ascan* normal) {

	std::vector<Ascan*>::iterator it;
	

	for (int i = 0; i < (int)scans.size(); i++) {
		int size = scans.at(i)->getSize();
		uint16_t* v = new uint16_t[size];

		for (unsigned int j = 0; j < normal->getSize(); j++) {
			int dif = scans.at(i)->getIndex(j) - normal->getIndex(j);

			v[j] = (uint16_t)(127*256 + dif);
		}
		
		Ascan* newScan = new Ascan(size, v);
		Ascan* old = scans.at(i);
		it = scans.begin();
		scans.erase(it + i);
		it = scans.begin();
		scans.insert(it + i, newScan);
		delete old;
		old = NULL;		
		

	}
}




int Bscan::Kernel(int dimension, unsigned int h, unsigned int d) {

	if (h < 1 || h > scans.size() - 2)
		return 0;
	if (d < 1 || d > scans.at(0)->getSize() - 2)
		return 0;

	//hardcoded to only be 3x3

	int yDif = 0;
	int xDif = 0;
	int items = 3 * 3;
	int* v = new int[items];

	
	// Y AXIS -------------------------------------------------

	/*					h

	0	1	2		+1	+2	+1
	3	4	5	->	 0	 0	 0		d
	6	7	8		-1	-2	-1

	*/
	Ascan* hn1 = scans.at(h - 1);
	Ascan* h0 = scans.at(h + 0);
	Ascan* hp1 = scans.at(h + 1);
	
	
	v[0] = +1 * hn1->getIndex(d - 1);
	v[1] = +2 * h0->getIndex(d - 1);
	v[2] = +1 * hp1->getIndex(d - 1);

	v[3] = 0;
	v[4] = 0;
	v[5] = 0;

	v[6] = -1 * hn1->getIndex(d + 1);
	v[7] = -2 * h0->getIndex(d + 1);
	v[8] = -1 * hp1->getIndex(d + 1);
	

	for (int i = 0; i < items; i++) {
		yDif += v[i];
	}
	yDif /= items;

	// X AXIS --------------------------------------------------

	/*					h

	0	1	2		+1	 0	-1
	3	4	5	->	+2	 0	-2		d
	6	7	8		+1	 0	-1

	*/
	/*

	v[0] = +1 * scans.at(h - 1)->getIndex(d - 1);
	v[1] =  0 * scans.at(h + 0)->getIndex(d - 1);
	v[2] = -1 * scans.at(h + 1)->getIndex(d - 1);

	v[3] = +2 * scans.at(h - 1)->getIndex(d + 0);
	v[4] =  0 * scans.at(h + 0)->getIndex(d + 0);
	v[5] = -2 * scans.at(h + 1)->getIndex(d + 0);

	v[6] = +1 * scans.at(h - 1)->getIndex(d + 1);
	v[7] =  0 * scans.at(h + 0)->getIndex(d + 1);
	v[8] = -1 * scans.at(h + 1)->getIndex(d + 1);


	for (int i = 0; i < items; i++) {
		xDif += v[i];
	}
	xDif /= items;

	*/

	yDif = (int)std::sqrt((yDif * yDif)*VERTICAL_GAIN + (xDif * xDif)*HORIZONTAL_GAIN) * KERNEL_GAIN;


	delete v;
	v = NULL;

	return yDif;
}