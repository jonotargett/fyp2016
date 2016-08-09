#pragma once
#include "HardwareInterface.h"

#include <algorithm>
#include <ctime>

#include "Log.h"



class DummyHardware :
	public HardwareInterface
{
private:
	Point realPosition;
	double realHeading;
	double realVelocity;

	double realSteeringAngle;
	double realThrottlePercentage;

	

	double random();

public:
	DummyHardware();
	~DummyHardware();

	bool initialise();
	bool updateLoop();

	Point getRealPosition();
	double getRealHeading();
	double getRealVelocity();
};



