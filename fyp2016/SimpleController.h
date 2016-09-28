#pragma once
#include "DriveController.h"
#include "Log.h"
#include "SimpleNavigator.h"

//#include "HRTimer.h"
//#include "Thread.h"
#include <thread>
#include <chrono>
#include <string>

#ifndef PI
#define PI 3.1415926535
#endif


/*enum NavState {
	NAV_CRUISE,
	NAV_TURNINBOUND,
	NAV_LANDMINE_DETECTED,
};*/


class SimpleController :
	public DriveController
{
private:
	bool enabled;

	unsigned int currentPathPoint;
	bool landmineDetected;
	int pathTravDir;
	NavState navState;

	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> end;

	std::thread* updater;
	bool alive;

	HardwareInterface* hwi;
	SimpleNavigator* ns;

public:
	SimpleController();
	~SimpleController();
	bool isAlive();

	bool initialise(HardwareInterface*, NavigationSystem*);
	void setEnabled(bool);
	bool isEnabled();
	void updateDynamics();
	void landMineDetected();

	int timer;

	bool updateLoop();

};