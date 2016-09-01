#pragma once
#include "HardwareInterface.h"

#include <algorithm>
#include <ctime>

#include "Log.h"

// want our updateloop to run at 60hz.
#define REFRESH_RATE 60
#ifndef PI
#define PI 3.1415926535
#endif


class DummyHardware :
	public HardwareInterface
{
public:
	DummyHardware();
	~DummyHardware();

	bool initialise();
	bool updateLoop();
	
	void setDesiredVelocity(double);
	void setDesiredSteeringAngle(double);
	void setDesiredThrottlePercentage(double);
	void setDesiredBrake(double);
	void setDesiredGear(Gear);
	void emergencyStop();
	
	Point getRealPosition();
	double getRealAbsoluteHeading();
	double getRealVelocity();
	double getRealSteeringAngle();
	double getRealThrottlePercentage();
	int getRealGear();
	double getRealBrakePercentage();

private:
	void update(double);
	void updateActuators(double);
	void updateVelocityActuators();
	double random();

	Point realPosition;
	double realAbsoluteHeading;
	double realVelocity;
	double realSteeringAngle;
	double realThrottlePercentage;
	Gear realGear;
	double realBrakePercentage;

	double desiredSteeringAngle;
	double desiredVelocity;
	double desiredThrottlePercentage;
	Gear desiredGear;
	double desiredBrakePercentage;

	double gearTimer;

};



