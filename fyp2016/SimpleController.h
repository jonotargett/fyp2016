#pragma once
#include "DriveController.h"
#include "Log.h"

//#include "HRTimer.h"
//#include "Thread.h"
#include <thread>
#include <chrono>
#include <string>

class SimpleController :
	public DriveController
{
private:
	bool enabled;

	unsigned int currentPathPoint = 0;
	bool landmineDetected = false;
	int pathTravDir = 1;
	std::string navState = "cruise";

	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> end;

	//Thread* updater;
	std::thread* updater;
	bool alive;

	HardwareInterface* hwi;
	NavigationSystem* ns;
public:
	SimpleController();
	~SimpleController();
	bool isAlive();

	bool initialise(HardwareInterface*, NavigationSystem*);
	void setEnabled(bool);
	bool isEnabled();
	void updateDynamics();
	void landMineDetected();

	bool updateLoop();

};

/*
class ControlUpdaterRunnable : public Runnable {
private:
	SimpleController* controller;
public:
	ControlUpdaterRunnable(SimpleController* c) : controller(c) {	}
	virtual void* run();
};
*/