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
	realAbsoluteHeading = 0.0;
	realVelocity = 0.0;
	realSteeringAngle = 0.0;
	realThrottlePercentage = 0.0;
	setGear(GEAR_NULL);

	desiredSteeringAngle = 0.0;
	desiredVelocity = 0.0;

	// return true once everything is initialised.
	// seeing as there is no actual hardware here,
	// just return true. oh yeah
	Log::d << "DummyHardware initialised." << std::endl;

	start();

	return true;
}

void DummyHardware::update() { // gets refreshed at 50Hz as defined by REFRESH_RATE
	//all hardware update logic goes here (not needed for real quad bike)

	// fake a slow merge towards actuator positions
	if (getSteeringAngle() < desiredSteeringAngle) realSteeringAngle += 0.008;
	if (getSteeringAngle() > desiredSteeringAngle) realSteeringAngle -= 0.008;

	// fake accelerating to throttle speed
	// each throttle percent corresponds to a certian speed given by throttleSpeed
	// not entirely realistic when on hills and stuff but for flat ground close enough
	// this will be handled by a proper controller with actual hardware
	if (getGear() == GEAR_FORWARD || getGear() == GEAR_REVERSE) {
		double throttleSpeed = 0.25 + 0.1475 * realThrottlePercentage;
		if (getVelocity() < throttleSpeed) realVelocity += 0.01;
		if (getVelocity() > throttleSpeed) realVelocity -= 0.01;
	}
	if (getGear() == GEAR_NEUTRAL) { // neutral
		realVelocity /= 1.2;
	}

	// brake stuff
	if (getBrake()) {
		realVelocity /= 1.6;
	}

	
	// calculating next position based on steer angle and velocity
	double distanceTravelled = realVelocity * 1 / REFRESH_RATE;
	double distanceForward = 0;
	double distanceRight = 0;
	double angleTurned = 0;
	
	if (realSteeringAngle == 0) {
		distanceForward = distanceTravelled;
	}
	else {
		double turnRadius = wheelBase / tan(-realSteeringAngle);
		angleTurned = distanceTravelled / turnRadius;
		distanceForward = turnRadius * sin(angleTurned);
		distanceRight = turnRadius - turnRadius * cos(angleTurned);
	}

	// updating position
	realPosition.x += distanceForward * sin(realAbsoluteHeading) + distanceRight * cos(realAbsoluteHeading);
	realPosition.y += distanceForward * cos(realAbsoluteHeading) - distanceRight * sin(realAbsoluteHeading);
	realAbsoluteHeading += angleTurned;
	
	while (realAbsoluteHeading > 3.14159265) realAbsoluteHeading -= 2 * 3.14159265;
	while (realAbsoluteHeading < -3.14159265) realAbsoluteHeading += 2 * 3.14159265;

	
	// return values back to the hardware interface, as if theyd been measured.
	setPosition(Point(realPosition.x + random()*0.0, realPosition.y + random()*0.0));
	setAbsoluteHeading(realAbsoluteHeading + random()*0.0);
	setVelocity(realVelocity + random()*0.0);
}


Point DummyHardware::getRealPosition() {
	return realPosition;
}
double DummyHardware::getRealAbsoluteHeading() {
	return realAbsoluteHeading;
}
double DummyHardware::getRealSteeringAngle() {
	return realSteeringAngle;
}
double DummyHardware::getRealThrottlePercentage() {
	return realThrottlePercentage;
}
double DummyHardware::getRealVelocity() {
	return realVelocity;
}

void DummyHardware::setDesiredSteeringAngle(double x) {
	if (x > 24 / 180 * 3.141592) x = 24 / 180 * 3.141592;
	if (x < -24 / 180 * 3.141592) x = -24 / 180 * 3.141592;
	desiredSteeringAngle = x;
}
void DummyHardware::setDesiredThrottlePercentage(double x) {
	setThrottlePercentage(x);
}
void DummyHardware::setDesiredBrake(bool x) {
	setBrake(x);
}
void DummyHardware::setDesiredGear(HardwareInterface::Gear x) {
	setGear(x);
}

// handles gear changes as well
void DummyHardware::setDesiredVelocity(double desiredVelocity) {
	if (desiredVelocity == 0) {
		setDesiredThrottlePercentage(0);
		setDesiredGear(GEAR_NEUTRAL);
		setDesiredBrake(true);
	}
	else if (desiredVelocity > 0) {
		// if we are travelling in the wrong direction
		if (getVelocity() < 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrake(true);
			return;
		}
		// if we are travelling in the correct direction
		else {
			setDesiredGear(GEAR_FORWARD);
			setDesiredBrake(false);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (desiredVelocity < idleSpeed) {
			if (getVelocity() < desiredVelocity) {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(false);
			}
			else {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(true);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large 
		// difference in actual speed and desired speed
		else {
			if (getVelocity() < desiredVelocity) {
				setDesiredThrottlePercentage(getThrottlePercentage() + 0.1);
			}
			if (getVelocity() > desiredVelocity) {
				setDesiredThrottlePercentage(getThrottlePercentage() - 0.4);
				//quad.setBrake(true);
			}
		}
	}
	else if (desiredVelocity < 0) {
		// if we are travelling in the wrong direction
		if (getVelocity() > 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrake(true);
			return;
		}
		// if we are travelling in the correct direction
		else {
			setDesiredGear(GEAR_REVERSE);
			setDesiredBrake(false);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (abs(desiredVelocity) < idleSpeed) {
			if (getVelocity() < desiredVelocity) {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(true);
			}
			else {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(false);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large
		// difference in actual speed and desired speed
		else {
			//REMEMBER WE'RE IN REVERSE HERE
			if (getVelocity() < desiredVelocity) {
				setDesiredThrottlePercentage(getThrottlePercentage() - 0.4);
			}
			if (getVelocity() > desiredVelocity) {
				setDesiredThrottlePercentage(getThrottlePercentage() + 0.1);
			}
		}
	}
}

bool DummyHardware::updateLoop() {

	std::chrono::time_point<std::chrono::high_resolution_clock> lastWindowUpdate = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> current;

	std::chrono::duration<double> seconds;

	while (isAlive()) { // loop continuously at REFRESH_RATE for constant speeds across machines

		current = std::chrono::high_resolution_clock::now();
		seconds = current - lastWindowUpdate;

		if (seconds.count() > (1.0 / REFRESH_RATE)) {
			lastWindowUpdate = current;
			update();
		}
		// no point blazing through this super fast
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}

	return true;
}