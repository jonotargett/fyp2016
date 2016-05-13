#pragma once


#include "Log.h"

#include "Window.h"
#include "HardwareInterface.h"
#include "DummyHardware.h"
#include "DriveController.h"
#include "SimpleController.h"
#include "FeatureDetector.h"
#include "NavigationSystem.h"
#include "SimpleNavigator.h"
#include "Communications.h"

class Overlord
{
private:
	bool initialised;

	Window* window;
	Communications* comms;
	HardwareInterface* hwi;
	DriveController* dc;
	NavigationSystem* ns;
	FeatureDetector* fd;

public:
	Overlord();
	~Overlord();

	bool initialise();

	void run();
};

