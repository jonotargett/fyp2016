#pragma once
#include "HardwareInterface.h"

#include <algorithm>
#include <ctime>

#include "Log.h"

// want our updateloop to run at 60hz.
#define REFRESH_RATE 60

class DummyHardware :
	public HardwareInterface
{
private:
	Point realPosition;
	double realAbsoluteHeading;
	double realVelocity;
	double realSteeringAngle;
	double realThrottlePercentage;

	double desiredSteeringAngle;
	double desiredVelocity;

	double random();

public:
	DummyHardware();
	~DummyHardware();

	bool initialise();
	bool updateLoop();
	void update();

	Point getRealPosition();
	double getRealAbsoluteHeading();
	double getRealVelocity();
	double getRealSteeringAngle();
	double getRealThrottlePercentage();

	void setDesiredVelocity(double);
	void setDesiredSteeringAngle(double);
	void setDesiredThrottlePercentage(double);
	void setDesiredBrake(bool);
	void setDesiredGear(Gear);
};



