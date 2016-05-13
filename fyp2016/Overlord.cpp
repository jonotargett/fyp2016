#include "Overlord.h"

using std::cout;
using std::endl;

Overlord::Overlord()
{
}


Overlord::~Overlord()
{
}


bool Overlord::initialise() {

	window = new Window();
	Log::d << "-> WINDOW DONE" << endl;

	comms = new Communications(2099);
	comms->initialise();
	Log::d << "-> COMMUNICATIONS DONE" << endl;

	hwi = new DummyHardware();
	hwi->initialise();
	hwi->start();
	Log::d << "-> HARDWARE INTERFACE DONE" << endl;

	dc = new SimpleController();
	dc->initialise(hwi);
	Log::d << "-> DRIVE CONTROLLER DONE" << endl;

	ns = new SimpleNavigator();
	ns->initialise(dc);
	Log::d << "-> NAVIGATION SYSTEM DONE" << endl;

	fd = new FeatureDetector(hwi);
	fd->initialise();
	fd->createImage(DISPLAY_KERNEL);
	Log::d << "-> FEATURE DETECTOR DONE" << endl;

	return true;
}


void Overlord::run() {
	window->update(fd->retrieveImage());




	while (!window->shouldQuit()) {

		window->handleEvents();
		// waste time
	}
}