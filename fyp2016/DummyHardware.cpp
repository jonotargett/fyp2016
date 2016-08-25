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

	double velocityChangeRate = 0.6;		// metres/second^2
	double frictionalDecayRate = 120;		// %/second??
	double brakingAcceleration = 50;		// m/s/s at 100% brake. interpolate inbetween

	
	double const positionAccuracy = 0.1;			// meters of spread each side of real value
	double const headingAccuracy = 0;			// radians of spread each side of real value
	double const velocityAccuracy = 0;			// m/s of spread each side of real value
	double const steeringAccuracy = 0;			// radians of spread each side of real value
	double const brakeAccuracy = 0;				// percent of spread each side of real value
	double const throttleAccuracy = 0;			// percent of spread each side of real value
	
/*
	double const positionAccuracy = 0;				// meters of spread each side of real value
	double const headingAccuracy = 0;				// radians of spread each side of real value
	double const velocityAccuracy = 0;				// m/s of spread each side of real value
	double const steeringAccuracy = 0;				// radians of spread each side of real value
	double const brakeAccuracy = 0;					// percent of spread each side of real value
	double const throttleAccuracy = 0;				// percent of spread each side of real value
*/
	updateActuators(time);
	setSteeringAngle(realSteeringAngle + random() * steeringAccuracy);
	setBrakePercentage(realBrakePercentage + random() * brakeAccuracy);
	setThrottlePercentage(realThrottlePercentage + random() * throttleAccuracy);
	setGear(realGear);

	// fake accelerating to throttle speed
	// each throttle percent corresponds to a certian speed given by throttleSpeed
	// not entirely realistic when on hills and stuff but for flat ground close enough
	// this will be handled by a proper controller with actual hardware
	
	if (realGear == GEAR_FORWARD || realGear == GEAR_REVERSE) {
		double throttleSpeed = 0.25 + 0.1475 * realThrottlePercentage;
		if (abs(realVelocity - throttleSpeed * realGear) < velocityChangeRate * time) realVelocity = throttleSpeed * realGear;
		if (realVelocity < throttleSpeed * realGear) realVelocity += velocityChangeRate * time;
		if (realVelocity > throttleSpeed * realGear) realVelocity -= velocityChangeRate * time;
	}
	if (realGear == GEAR_NEUTRAL) {
		realVelocity /= (1 + (frictionalDecayRate*time)/100.0);
		if (abs(realVelocity) < 0.05) realVelocity = 0;
	}

	// brake stuff
	if (abs(realVelocity) < (realBrakePercentage / 100) * brakingAcceleration * time)
		realVelocity = 0;
	if (realVelocity > 0)
		realVelocity -= (realBrakePercentage/100) * brakingAcceleration * time;
	if (realVelocity < 0)
		realVelocity += (realBrakePercentage / 100) * brakingAcceleration * time;

	// calculating next position based on steer angle and velocity
	double distanceTravelled = realVelocity * time;
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
	setPosition(Point(realPosition.x + random() * positionAccuracy, realPosition.y + random() * positionAccuracy));
	setAbsoluteHeading(realAbsoluteHeading + random() * headingAccuracy);
	setVelocity(realVelocity + random() * velocityAccuracy);
}

void DummyHardware::updateActuators(double time) {
	// all actuator update logic goes here (not needed for real quad bike)
	// fake a slow merge towards actuator positions

	double steerChangeRate = 0.5;			// radians/second
	double gearChangeRate = 1;				// second / gear change
	double throttleChangeRate = 25;			// percentChange/second
	double brakeChangeRate = 20;			// percentChange/second

	// steering

	if (getSteeringAngle() < desiredSteeringAngle) realSteeringAngle += steerChangeRate * time;
	if (getSteeringAngle()  > desiredSteeringAngle) realSteeringAngle -= steerChangeRate * time;
	//if (abs(getSteeringAngle() - desiredSteeringAngle) < steerChangeRate * time) realSteeringAngle = desiredSteeringAngle;

	// throttle
	if (getThrottlePercentage() < desiredThrottlePercentage) realThrottlePercentage += throttleChangeRate * time;
	if (getThrottlePercentage() > desiredThrottlePercentage) realThrottlePercentage -= throttleChangeRate * time;
	if (abs(getThrottlePercentage() - desiredThrottlePercentage) < throttleChangeRate * time) realThrottlePercentage = desiredThrottlePercentage;

	// brake
	if (getBrakePercentage() < desiredBrakePercentage) realBrakePercentage += brakeChangeRate * time;
	if (getBrakePercentage() > desiredBrakePercentage) realBrakePercentage -= brakeChangeRate * time;

	// gear
	if (gearTimer > 0) gearTimer -= time;
	if (gearTimer <= 0) realGear = desiredGear;

}

void DummyHardware::setDesiredSteeringAngle(double x) {
	double pi = PI;
	double limit = 24 * pi / 180;

	if (x > limit) x = limit;
	if (x < -limit) x = -limit;
	desiredSteeringAngle = x;
}
void DummyHardware::setDesiredThrottlePercentage(double x) {
	if (x > 100) x = 100;
	if (x < 0) x = 0;
	desiredThrottlePercentage = x;
}
void DummyHardware::setDesiredBrake(double x) {
	if (x > 100) x = 100;
	if (x < 0) x = 0;
	desiredBrakePercentage = x;
}
void DummyHardware::setDesiredGear(HardwareInterface::Gear x) {
	if (gearTimer <= 0 && realGear != x)
		gearTimer = 0.5; // seconds per gear change
	
	desiredGear = x;
}

void DummyHardware::setDesiredVelocity(double x) {
	desiredVelocity = x;
}

// handles gear changes as well
void DummyHardware::updateVelocityActuators() {

	double throttlePercentageRequired = (abs(desiredVelocity) - 0.25) / 0.1475;

	if (desiredVelocity == 0) {
		setDesiredThrottlePercentage(0);
		setDesiredGear(GEAR_NEUTRAL);
		setDesiredBrake(15);
	}
	else if (desiredVelocity > 0) {
		// if we are travelling in the wrong direction
		if (getVelocity() < 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrake(15);
			return;
		}
		// if we are travelling in the correct direction
		else {
			setDesiredGear(GEAR_FORWARD);
			setDesiredBrake(0);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (desiredVelocity < idleSpeed) {
			if (getVelocity()  < desiredVelocity) {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(0);
			}
			else {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(15);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large 
		// difference in actual speed and desired speed
		else {
			/*if (getVelocity()  < desiredVelocity) {
				setDesiredThrottlePercentage(desiredThrottlePercentage + 0.05);
			}
			if (getVelocity()  > desiredVelocity) {
				setDesiredThrottlePercentage(desiredThrottlePercentage - 0.05);
			}*/
			setDesiredThrottlePercentage(throttlePercentageRequired);
		}
	}
	else if (desiredVelocity < 0) {
		// if we are travelling in the wrong direction
		if (getVelocity()  > 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrake(15);
			return;
		}
		// if we are travelling in the correct direction
		else {
			setDesiredGear(GEAR_REVERSE);
			setDesiredBrake(0);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (abs(desiredVelocity) < idleSpeed) {
			if (getVelocity()  < desiredVelocity) {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(15);
			}
			else {
				setDesiredThrottlePercentage(0);
				setDesiredBrake(0);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large
		// difference in actual speed and desired speed
		else {
			
			//REMEMBER WE'RE IN REVERSE HERE
			/*if (getVelocity()  < desiredVelocity) {
				setDesiredThrottlePercentage(desiredThrottlePercentage - 0.05);
			}
			if (getVelocity()  > desiredVelocity) {
				setDesiredThrottlePercentage(desiredThrottlePercentage + 0.05);
			}*/
			setDesiredThrottlePercentage(throttlePercentageRequired);
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

double DummyHardware::getRealBrakePercentage() {
	return realBrakePercentage;
}