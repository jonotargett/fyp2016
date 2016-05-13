#pragma once

#include "Log.h"
#include "SDL\SDL.h"

#include "Point.h"
#include "Runnable.h"
#include "Thread.h"

class HardwareInterface
{
private:
	bool alive;
	/*
	variables holding sensor data, actuator data
	*/
	Point position;
	double vehicleAbsoluteHeading;
	double velocity;
	double velocityAbsoluteHeading;

	double steeringAngle;
	double throttlePercentage;

protected:
	Thread* updater;

	/*
	Locks for each of the sensor variables
	*/
	bool positionLock;
	bool vehicleAbsoluteHeadingLock;
	bool velocityLock;
	bool velocityAbsoluteHeadingLock;
	bool steeringAngleLock;
	bool throttlePercentageLock;
	
public:
	HardwareInterface();
	virtual ~HardwareInterface();
	bool isAlive();

	/*
	Obtain a link with the microcontroller over the serial link.
	Returns: true on success, false on failure
	*/
	virtual bool initialise() = 0;

	/* 
	is declared public only so that it can be called on a
	separate thread. there is no reason to call this method externally.
	This method gets/sets all data to the microcontroller
	each pass returns 'true' for success, or 'false' for error.
	*/
	virtual bool updateLoop() = 0;

	bool start();
	void stop();

	/* 
	Returns: the current position as determined by the GPS unit
	*/
	Point getPosition();
	void setPosition(Point p);

	double getAbsoluteHeading();
	void setAbsoluteHeading(double);

	double getVelocity();
	void setVelocity(double);

	double getVelocityHeading();
	void setVelocityHeading(double);

	double getSteeringAngle();
	void setSteeringAngle(double);

	double getThrottlePercentage();
	void setThrottlePercentage(double);

};


class UpdaterRunnable : public Runnable {
private:
	HardwareInterface* hwi;
public:
	UpdaterRunnable(HardwareInterface* i) : hwi(i) {	}
	virtual void* run();
};

