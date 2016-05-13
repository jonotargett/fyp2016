#pragma once

#include "DriveController.h"

#include "Log.h"

#include "HRTimer.h"
#include "Thread.h"

class SimpleController :
	public DriveController
{
private:
	HardwareInterface* hwi;
	bool enabled;

	double heading;
	double dist;
	double timeLast;
	HRTimer hrt;

	Thread* updater;
	bool alive;
public:
	SimpleController();
	~SimpleController();
	bool isAlive();

	bool initialise(HardwareInterface*);
	void setEnabled(bool);
	bool isEnabled();
	bool setInputs(double, double);

	bool updateLoop();
};


class ControlUpdaterRunnable : public Runnable {
private:
	SimpleController* controller;
public:
	ControlUpdaterRunnable(SimpleController* c) : controller(c) {	}
	virtual void* run();
};