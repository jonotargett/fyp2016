#pragma once

#include "Log.h"

#include "Point.h"
//#include "Thread.h"
//#include "HRTimer.h"

#include <thread>
#include <chrono>

enum Gear
{
	GEAR_FORWARD = 1,
	GEAR_NEUTRAL = 0,
	GEAR_REVERSE = -1,
	GEAR_NULL = NULL
};



class HardwareInterface
{

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
	Returns: the current value as stored in this class
	*/
	Point getPosition();
	double getAbsoluteHeading();
	double getVelocity();
	Gear getGear();
	double getBrakePercentage();
	double getSteeringAngle();
	double getThrottlePercentage();
	
	virtual void setDesiredVelocity(double);
	virtual void setDesiredSteeringAngle(double);
	virtual void setDesiredThrottlePercentage(double);
	virtual void setDesiredBrakePercentage(double);
	virtual void setDesiredGear(Gear);
	virtual void updateVelocityActuators();
	virtual void emergencyStop() = 0;

	const double width = 1.18;
	const double length = 1.86;
	const double wheelBase = 1.28;
	const double overHang = (length - wheelBase) / 2;
	const double wheelRadius = 0.3;
	const double wheelWidth = 0.25;
	const double maxSteerAngle = 24 * 3.141592 / 180;
	const double cruiseVelocity = 1.5;
	const double idleSpeed = 0.25;

private:
	
	bool alive;
	/*
	variables holding sensor data, actuator data
	*/

	Point position;
	double absoluteHeading;
	double velocity;

	double steeringAngle;
	double throttlePercentage;
	double brakePercentage;
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
	void setBrakePercentage(double);
	void setSteeringAngle(double);
	void setThrottlePercentage(double);


	/*
	Locks for each of the sensor variables
	*/
	bool positionLock;
	bool absoluteHeadingLock;
	bool velocityLock;
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
