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

	realPosition = Point(0, 0);
	realHeading = 0.0;
	realVelocity = 0.0;
	realSteeringAngle = 0.0;
	realThrottlePercentage = 0.0;
	setGear(GEAR_NULL);

	// return true once everything is initialised.
	// seeing as there is no actual hardware here,
	// just return true. oh yeah
	Log::d << "DummyHardware initialised." << std::endl;

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

		realVelocity = 0.1 * (realVelocity + (realThrottlePercentage / 2.0))*(1 - seconds.count());
		realPosition = Point(realPosition.x, realPosition.y + realVelocity * seconds.count());

		// return values back to the hardware interface, as if theyd been measured.
		setPosition(Point(realPosition.x + random()*0.5, realPosition.y + random()*0.5));
		setAbsoluteHeading(realHeading + 5 * random());
		setVelocity(realVelocity + 0.2*random());

		startTime = endTime;

		//Cap at 1000Hz. No point blazing through this any faster
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	
	return true;
}


// handles gear changes as well
void DummyHardware::setDesiredVelocity(double desiredVelocity) {
	if (desiredVelocity == 0) {
		setThrottlePercentage(0);
		setGear(GEAR_NEUTRAL);
		setBrake(true);
	}
	else if (desiredVelocity > 0) {
		// if we are travelling in the wrong direction
		if (getVelocity() < 0) {
			quad.setThrottlePercentage(0);
			quad.setGear(0);
			quad.setBrake(true);
			return;
		}
		// if we are travelling in the correct direction
		else {
			quad.setGear(1);
			quad.setBrake(false);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (desiredVelocity < quad.getIdleSpeed()) {
			if (quad.getVelocity() < desiredVelocity) {
				quad.setThrottlePercentage(0);
				quad.setBrake(false);
			}
			else {
				quad.setThrottlePercentage(0);
				quad.setBrake(true);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large 
		// difference in actual speed and desired speed
		else {
			if (getVelocity() < desiredVelocity) {
				setThrottlePercentage(quad.getThrottle() + 0.1);
			}
			if (getVelocity() > desiredVelocity) {
				setThrottlePercentage(quad.getThrottle() - 0.4);
				//quad.setBrake(true);
			}
		}
	}
	else if (desiredVelocity < 0) {
		// if we are travelling in the wrong direction
		if (getVelocity() > 0) {
			setThrottlePercentage(0);
			setGear(GEAR_NEUTRAL);
			setBrake(true);
			return;
		}
		// if we are travelling in the correct direction
		else {
			setGear(GEAR_REVERSE);
			setBrake(false);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (abs(desiredVelocity) < quad.getIdleSpeed()) {
			if (getVelocity() < desiredVelocity) {
				setThrottlePercentage(0);
				setBrake(true);
			}
			else {
				setThrottlePercentage(0);
				setBrake(false);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large
		// difference in actual speed and desired speed
		else {
			//REMEMBER WE'RE IN REVERSE HERE
			if (getVelocity() < desiredVelocity) {
				setThrottlePercentage(quad.getThrottle() - 0.4);
			}
			if (getVelocity() > desiredVelocity) {
				setThrottlePercentage(quad.getThrottle() + 0.1);
			}
		}
	}
}