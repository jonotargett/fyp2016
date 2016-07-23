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

	fd = new FeatureDetector(hwi);
	fd->initialise();
	Log::d << "-> FEATURE DETECTOR DONE" << endl;

	vp = new VirtualPlatform();
	vp->initialise(ns, window->getRenderer());
	Log::d << "-> VIRTUAL PLATFORM DONE" << endl;
	
	initialised = true;
	return true;
}


void Overlord::run() {
	

	//fd->createImage(DISPLAY_KERNEL);
	
	window->showWindow(true);

	//window->update(fd->retrieveImage());
	//window->update(vp->getTexture());

	Log::setVerbosity(LOG_INFORMATIVE);

	vp->drawTexture();
	window->update(vp->getTexture());
	
	while (!window->shouldQuit()) {

		window->handleEvents();
		// waste time

		SDL_Delay(50);
	}
}