#pragma once

#include "Log.h"

#include "Point.h"
//#include "Thread.h"
//#include "HRTimer.h"

#include <thread>
#include <chrono>

class HardwareInterface
{

public:
	HardwareInterface();
	virtual ~HardwareInterface();
	bool isAlive();

	enum Gear
	{
		GEAR_FORWARD,
		GEAR_NEUTRAL,
		GEAR_REVERSE,
		GEAR_NULL
	};

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
	Returns: the current value as stored in this class
	*/
	Point getPosition();
	double getAbsoluteHeading();
	double getVelocity();
	Gear getGear();
	bool getBrake();
	double getVelocityHeading();
	double getSteeringAngle();
	double getThrottlePercentage();
	
	virtual void setDesiredVelocity(double) = 0;
	virtual void setDesiredSteeringAngle(double) = 0;
	virtual void setDesiredThrottlePercentage(double) = 0;
	virtual void setDesiredBrake(bool) = 0;
	virtual void setDesiredGear(Gear) = 0;

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
	bool brake;
	Gear gear;

protected:
	//Thread* updater;
	std::thread* updater;
	//HRTimer hrt;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime;


	/*
	Setters: these are used by the INHERITED CLASSES. these set the values
	inside this class. the reason the setters exist is to prevent direct access
	to the variables. Direct access could cause concurrent thread violations.
	*/
	void setPosition(Point p);
	void setAbsoluteHeading(double);
	void setVelocity(double);
	void setGear(Gear);
	void setBrake(bool);
	void setVelocityHeading(double);
	void setSteeringAngle(double);
	void setThrottlePercentage(double);


	/*
	Locks for each of the sensor variables
	*/
	bool positionLock;
	bool vehicleAbsoluteHeadingLock;
	bool velocityLock;
	bool velocityAbsoluteHeadingLock;
	bool steeringAngleLock;
	bool throttlePercentageLock;
	bool gearLock;
	bool brakeLock;


};

/*
class UpdaterRunnable : public Runnable {
private:
	HardwareInterface* hwi;
public:
	UpdaterRunnable(HardwareInterface* i) : hwi(i) {	}
	virtual void* run();
};
*/
