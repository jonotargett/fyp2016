#include <iostream>

#include "Window.h"

using namespace std;

int main(int argc, char **argv) {

	Window* window = new Window();

	Bscan* scan = new Bscan();
	Bscan* empty = new Bscan();

	/*
	FILENAMES:

	01011329.DAT	- empty
	01011330.DAT	- empty
	01011348.DAT	- empty
	01041445.DAT	- ST-AT(1)-flat-at0.5cm Pepsi-perp-at0.5cm
	01031453.DAT	- filledPepsi-perp-at0.5cm Pepsi-flat(upright)-at0.5cm
	01021000.DAT	- PVC10x15-para-at0.5cm SS10x05-at0.5cm
	*/

	empty->load("C:/Users/Jono/Documents/Visual Studio 2015/Projects/fyp2016/Debug/01011329.DAT");
	scan->load("C:/Users/Jono/Documents/Visual Studio 2015/Projects/fyp2016/Debug/01021000.DAT");

	Ascan* normal = scan->produceNormal(scan->length());
	scan->normalise(normal);

	window->setBscan(scan);

	window->update();

	std::cout << scan->length() << std::endl;

	while (!window->shouldQuit()) {

		window->handleEvents();
		// waste time
	}

	return 0;
}