#include <iostream>

#include "Window.h"
#include "HardwareInterface.h"
#include "DummyHardware.h"
#include "DriveController.h"
#include "SimpleController.h"
#include "FeatureDetector.h"
#include "NavigationSystem.h"
#include "SimpleNavigator.h"
#include "Communications.h"

using namespace std;

int main(int argc, char **argv) {

	Window* window = new Window();
	cout << "-> WINDOW DONE" << endl;

	Communications* comms = new Communications(2099);
	comms->initialise();
	cout << "-> COMMUNICATIONS DONE" << endl;
	
	HardwareInterface* hwi = new DummyHardware();
	hwi->initialise();
	hwi->start();
	cout << "-> HARDWARE INTERFACE DONE" << endl;

	DriveController* dc = new SimpleController();
	dc->initialise(hwi);
	cout << "-> DRIVE CONTROLLER DONE" << endl;

	NavigationSystem* ns = new SimpleNavigator();
	ns->initialise(dc);
	cout << "-> NAVIGATION SYSTEM DONE" << endl;

	FeatureDetector* fd = new FeatureDetector(hwi);
	fd->initialise();
	fd->createImage(DISPLAY_RAW);
	cout << "-> FEATURE DETECTOR DONE" << endl;
	

	window->update(fd->retrieveImage());

	while (!window->shouldQuit()) {

		window->handleEvents();
		// waste time
	}

	return 0;
}