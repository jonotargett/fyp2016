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
#include "VirtualPlatform.h"
#include "CommsListener.h"


#define DATA_REFRESH_RATE 100
#define VIEW_REFRESH_RATE 25


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
	VirtualPlatform* vp;

	std::chrono::time_point<std::chrono::high_resolution_clock> lastDataUpdate;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastWindowUpdate;
	std::chrono::time_point<std::chrono::high_resolution_clock> current;
};

