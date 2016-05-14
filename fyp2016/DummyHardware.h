#pragma once
#include "HardwareInterface.h"

#include <algorithm>
#include <ctime>

#include "Log.h"

//#include "HRTimer.h"

class DummyHardware :
	public HardwareInterface
{
private:
	Point realPosition;
	double realHeading;
	double realVelocity;

	double realSteeringAngle;
	double realThrottlePercentage;

	//HRTimer hrt;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

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



