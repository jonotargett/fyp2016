#pragma once

#include "Log.h"
#include "Window.h"
#include "HardwareInterface.h"
#include "DummyHardware.h"
#include "QuadInterface.h"
#include "DriveController.h"
#include "SimpleController.h"
#include "FeatureDetector.h"
#include "NavigationSystem.h"
#include "SimpleNavigator.h"
#include "Communications.h"
#include "VirtualPlatformOld.h"
#include "VirtualPlatform.h"
#include "CommsListener.h"

#define REFRESH_RATE 60


class Overlord :
	public CommsListener
{
public:
	Overlord();
	~Overlord();

	bool initialise();
	void run();
	virtual void onEvent(Packet*);
	void handleEvents();

private:
	bool initialised;
	bool showvp;

	Window* window;
	Communications* comms;
	HardwareInterface* hwi;
	HardwareInterface* dhwi;
	DriveController* dc;
	NavigationSystem* ns;
	FeatureDetector* fd;
	VirtualPlatformOld* vpo;
	VirtualPlatform* vp;

	
	std::chrono::time_point<std::chrono::high_resolution_clock> lastWindowUpdate;
	std::chrono::time_point<std::chrono::high_resolution_clock> current;
};

