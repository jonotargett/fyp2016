#pragma once
#include "DriveController.h"
#include "Log.h"

//#include "HRTimer.h"
//#include "Thread.h"
#include <thread>
#include <chrono>
#include <string>

#define	width 1.18
#define length 1.86
#define wheelBase 1.28
#define overHang (length - wheelBase) / 2
#define wheelRadius 0.3
#define wheelWidth 0.25
#define maxSteerAngle 24 * 3.141592 / 180
#define cruiseVelocity 1.2
#define idleSpeed 0.25

class SimpleController :
	public DriveController
{
private:
	HardwareInterface* hwi;
	NavigationSystem* ns;
	bool enabled;

	unsigned int currentPathPoint = 0;
	double desiredVelocity = 0;
	bool landmineDetected = false;
	double distanceSinceMine = 0;
	int pathTravDir = 1;
	void updateDynamics();
	std::string navState = "cruise";

	double heading;
	double dist;
	double timeLast;
	//HRTimer hrt;
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> end;

	//Thread* updater;
	std::thread* updater;
	bool alive;
public:
	SimpleController();
	~SimpleController();
	bool isAlive();

	bool initialise(HardwareInterface*, NavigationSystem*);
	void setEnabled(bool);
	bool isEnabled();

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