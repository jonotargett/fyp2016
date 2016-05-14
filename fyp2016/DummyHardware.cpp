#include "DummyHardware.h"



DummyHardware::DummyHardware()
{
}


DummyHardware::~DummyHardware()
{
}

double DummyHardware::random() {
	return (((double)rand() / (RAND_MAX)) - 0.5) * 2.0;
}


bool DummyHardware::initialise() {
	// TODO: init whatever

	srand((unsigned int)time(NULL));
	//hrt = HRTimer();
	startTime = std::chrono::high_resolution_clock::now();

	realPosition = Point();
	realHeading = 0.0;
	realVelocity = 0.0;
	realSteeringAngle = 0.0;
	realThrottlePercentage = 0.0;

	// return true once everything is initialised.
	// seeing as there is no actual hardware here,
	// just return true. oh yeah
	Log::i << "DummyHardware initialised." << std::endl;

	start();

	return true;
}

bool DummyHardware::updateLoop() {
	while (isAlive()) {

		// loop continuously
		//get loop time
		//double seconds = hrt.getElapsedTimeSeconds();
		//hrt.reset
		endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> seconds = endTime - startTime;

		// fake a slow merge towards actuator positions

		realSteeringAngle = (getSteeringAngle() + realSteeringAngle) / 2.0;
		realThrottlePercentage = (getThrottlePercentage() + realThrottlePercentage) / 2.0;

		// implement motion - faked

		double bikeLength = 1.8212;		// 71.7 inches
		double bikeWidth = 1.1557;		// 45.5 inches
		double wheelbase = 1.2166;		// 47.9 inches
		double turningRadius = 3.2004;	// 10.5 feet

		realVelocity = (realVelocity + (realThrottlePercentage / 2.0))*(1 - seconds.count());

		// return values back to the hardware interface, as if theyd been measured.

		setPosition(realPosition + Point(random()*0.5, random()*0.5));
		setAbsoluteHeading(realHeading + 5 * random());
		setVelocity(realVelocity + 0.2*random());

		startTime = endTime;

		//Cap at 1000Hz. No point blazing through this any faster
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	
	return true;
}