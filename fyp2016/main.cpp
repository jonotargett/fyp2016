#include <iostream>

#include "Window.h"
#include "HardwareInterface.h"
#include "FeatureDetector.h"
#include "NavigationSystem.h"

using namespace std;

int main(int argc, char **argv) {

	Window* window = new Window();

	
	HardwareInterface* hwi = new HardwareInterface();
	hwi->initialise();

	

	while (!window->shouldQuit()) {

		window->handleEvents();
		// waste time
	}

	return 0;
}