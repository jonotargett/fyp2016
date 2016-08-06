#include "Overlord.h"

#include "Graph.h"

Overlord::Overlord()
{
	initialised = false;
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
	Log::d << "-> HARDWARE INTERFACE DONE" << endl;

	dc = new SimpleController();
	dc->initialise(hwi);
	Log::d << "-> DRIVE CONTROLLER DONE" << endl;

	ns = new SimpleNavigator();
	ns->initialise(dc, hwi);
	Log::d << "-> NAVIGATION SYSTEM DONE" << endl;

	//fd = new FeatureDetector(hwi, window->getRenderer());
	//fd->initialise();
	//Log::d << "-> FEATURE DETECTOR DONE" << endl;

	vp = new VirtualPlatform();
	vp->initialise(ns, window->getRenderer());
	Log::d << "-> VIRTUAL PLATFORM DONE" << endl;

	initialised = true;
	return true;
}


void Overlord::run() {

	window->showWindow(true);

	// Feature detector stuff
	//fd->loadScan();
	//fd->createImage(DISPLAY_KERNEL);
	//window->update(fd->retrieveImage());

	
	Log::setVerbosity(LOG_INFORMATIVE);

	vp->update();
	vp->drawTexture();

	while (!window->shouldQuit()) {

		window->handleEvents();
		// waste time

		// Virtual platform stuff
		vp->update();
		vp->drawTexture();
		window->update(vp->retrieveImage());

		//fd->runScan();
		//fd->createImage(DISPLAY_RAW);
		//window->update(fd->retrieveImage());

		SDL_Delay(5);
	}
}