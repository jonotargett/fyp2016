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

	// this is allowed to be magic numbers. nothing else should be though
	realPosition = Point(-2.05, -4);

	realAbsoluteHeading = 0.0;
	realVelocity = 0.0;
	realSteeringAngle = 0.0 * PI / 180;
	realThrottlePercentage = 0.0;
	realGear = GEAR_NEUTRAL;

	desiredSteeringAngle = 0.0;
	desiredVelocity = 0.0;

	// return true once everything is initialised.
	// seeing as there is no actual hardware here,
	// just return true. oh yeah
	Log::d << "DummyHardware initialised." << std::endl;

	start();

	return true;
}

void DummyHardware::update(double time) { // gets refreshed at 50Hz as defined by REFRESH_RATE

	double steerChangeRate = 0.36;			// radians/second
	double velocityChangeRate = 0.6;		// metres/second^2
	double frictionalDecayRate = 120;		// %/second??
	double brakingDecayRate = 480;			// %/second??

	//all hardware update logic goes here (not needed for real quad bike)
	// fake a slow merge towards actuator positions
	if (realSteeringAngle < desiredSteeringAngle) realSteeringAngle += steerChangeRate * time;
	if (realSteeringAngle > desiredSteeringAngle) realSteeringAngle -= steerChangeRate * time;

	// fake accelerating to throttle speed
	// each throttle percent corresponds to a certian speed given by throttleSpeed
	// not entirely realistic when on hills and stuff but for flat ground close enough
	// this will be handled by a proper controller with actual hardware
	if (realGear == GEAR_FORWARD || realGear == GEAR_REVERSE) {
		double throttleSpeed = 0.25 + 0.1475 * realThrottlePercentage;
		if (realVelocity < throttleSpeed * realGear) realVelocity += velocityChangeRate * time;
		if (realVelocity > throttleSpeed * realGear) realVelocity -= velocityChangeRate * time;
	}
	if (realGear == GEAR_NEUTRAL) { // neutral
		realVelocity /= (1 + (frictionalDecayRate*time)/100.0);
	}

	// brake stuff
	if (realBrake) {
		realVelocity /= (1 + (brakingDecayRate*time) / 100.0);
	}


	//rounding stuff that needs to be done. wont have an issue in real life with this
	if (abs(realVelocity - desiredVelocity) < 0.01) realVelocity = desiredVelocity;
	if (abs(realVelocity) < 0.01) realVelocity = 0;


	
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
	
	while (realAbsoluteHeading > PI) realAbsoluteHeading -= 2 * PI;
	while (realAbsoluteHeading < -PI) realAbsoluteHeading += 2 * PI;

	// this keeps the quad going at a constant speed given by desiredVelocity.
	updateVelocityActuators();


	// return values back to the hardware interface, as if theyd been measured.
	setPosition(Point(realPosition.x, realPosition.y));
	setAbsoluteHeading(realAbsoluteHeading);
	setVelocity(realVelocity);
	setSteeringAngle(realSteeringAngle);
}

void DummyHardware::setDesiredSteeringAngle(double x) {
	double pi = PI;
	double limit = 24 * pi / 180;

	if (x > limit) x = limit;
	if (x < -limit) x = -limit;
	desiredSteeringAngle = x;
}
void DummyHardware::setDesiredThrottlePercentage(double x) {
	setThrottlePercentage(x);
	// what is the delay for this? IRL wont be an instantaneous change
	realThrottlePercentage = x;
}
void DummyHardware::setDesiredBrake(bool x) {
	setBrake(x);
	// what is the delay for this? IRL wont be an instantaneous change
	realBrake = x;
}
void DummyHardware::setDesiredGear(HardwareInterface::Gear x) {
	setGear(x);
	// what is the delay for this? IRL wont be an instantaneous change
	realGear = x;
}

void DummyHardware::setDesiredVelocity(double d) {
	desiredVelocity = d;
}

// handles gear changes as well
void DummyHardware::updateVelocityActuators() {
	if (desiredVelocity == 0) {
		setDesiredThrottlePercentage(0);
		setDesiredGear(GEAR_NEUTRAL);
		setDesiredBrake(true);
	}
	else if (desiredVelocity > 0) {
		// if we are travelling in the wrong direction
		if (realVelocity < 0) {
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
			if (realVelocity < desiredVelocity) {
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
			if (realVelocity < desiredVelocity) {
				setDesiredThrottlePercentage(realThrottlePercentage + 0.1);
			}
			if (realVelocity > desiredVelocity) {
				setDesiredThrottlePercentage(realThrottlePercentage - 0.4);
			}
		}
	}
	else if (desiredVelocity < 0) {
		// if we are travelling in the wrong direction
		if (realVelocity > 0) {
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
			if (realVelocity < desiredVelocity) {
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
			if (realVelocity < desiredVelocity) {
				setDesiredThrottlePercentage(realThrottlePercentage - 0.4);
			}
			if (realVelocity > desiredVelocity) {
				setDesiredThrottlePercentage(realThrottlePercentage + 0.1);
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
			update(seconds.count());
		}
		// no point blazing through this super fast
		std::this_thread::sleep_for(std::chrono::microseconds(500));
	}

	return true;
}




Point DummyHardware::getRealPosition() {
	return realPosition;
}

double DummyHardware::getRealAbsoluteHeading() {
	return realAbsoluteHeading;
}

double DummyHardware::getRealVelocity() {
	return realVelocity;
}

double DummyHardware::getRealSteeringAngle() {
	return realSteeringAngle;
}

double DummyHardware::getRealThrottlePercentage() {
	return realThrottlePercentage;
}

int DummyHardware::getRealGear() {
	return realGear;
}

bool DummyHardware::getRealBrake() {
	return realBrake;
}