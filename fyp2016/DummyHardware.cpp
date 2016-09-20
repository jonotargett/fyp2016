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

	velocityChangeRate = 0.6;		// metres/second^2
	frictionalDecayRate = 120;		// %/second??
	brakingAcceleration = 50;		// m/s/s at 100% brake. interpolate inbetween

	positionPrecision = 0.5;		// meters of spread each side of real value
	driftSpeed = 0.2;				// meters of drift per second
	headingAccuracy = 0;			// radians of spread each side of real value (this wont be a thing, will come from kalman filter)
	velocityAccuracy = 0.5;			// m/s of spread each side of real value
	steeringAccuracy = 0.005;		// radians of spread each side of real value
	brakeAccuracy = 0;				// percent of spread each side of real value
	throttleAccuracy = 0;			// percent of spread each side of real value
	gpsAccuracy = 2;				// meters spread each side of real value

	/*
	double const positionAccuracy = 0;				// meters of spread each side of real value
	double const headingAccuracy = 0;				// radians of spread each side of real value
	double const velocityAccuracy = 0;				// m/s of spread each side of real value
	double const steeringAccuracy = 0;				// radians of spread each side of real value
	double const brakeAccuracy = 0;					// percent of spread each side of real value
	double const throttleAccuracy = 0;				// percent of spread each side of real value
	*/

	srand((unsigned int)time(NULL));
	//hrt = HRTimer();
	startTime = std::chrono::high_resolution_clock::now();

	realPosition = Point(0, 0);
	kinematicPosition = realPosition;
	accelerometerPosition = realPosition;
	gpsPosition = realPosition;
	oldgpsPosition = realPosition;
	setPosition(realPosition);

	realAbsoluteHeading = 10.0 * PI / 180;
	kinematicHeading = realAbsoluteHeading;
	realVelocity = 0.0;
	realSteeringAngle = 0.0;
	realThrottlePercentage = 0.0;
	realGear = GEAR_NEUTRAL;

	desiredSteeringAngle = 0.0;
	desiredVelocity = 0.0;
	timeSinceLastGpsUpdate = 0.0;

	setUpKalmanMatrices();


	// return true once everything is initialised.
	// seeing as there is no actual hardware here,
	// just return true. oh yeah
	Log::d << "DummyHardware initialised." << std::endl;

	start();

	return true;
}

void DummyHardware::update(double time) { // gets refreshed at 50Hz as defined by REFRESH_RATE

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
		double throttleSpeed = 0.25 + 0.05 * realThrottlePercentage;
		if (abs(realVelocity - throttleSpeed * realGear) < velocityChangeRate * time) realVelocity = throttleSpeed * realGear;
		if (realVelocity < throttleSpeed * realGear) realVelocity += velocityChangeRate * time;
		if (realVelocity > throttleSpeed * realGear) realVelocity -= velocityChangeRate * time;
	}
	if (realGear == GEAR_NEUTRAL) {
		realVelocity /= (1 + (frictionalDecayRate*time) / 100.0);
		if (abs(realVelocity) < 0.05) realVelocity = 0;
	}

	// brake stuff
	if (abs(realVelocity) < (realBrakePercentage / 100) * brakingAcceleration * time)
		realVelocity = 0;
	if (realVelocity > 0)
		realVelocity -= (realBrakePercentage / 100) * brakingAcceleration * time;
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
	Point oldPosition = realPosition;
	realPosition.x += distanceForward * sin(realAbsoluteHeading) + distanceRight * cos(realAbsoluteHeading);
	realPosition.y += distanceForward * cos(realAbsoluteHeading) - distanceRight * sin(realAbsoluteHeading);
	realAbsoluteHeading += angleTurned;

	while (realAbsoluteHeading > PI) realAbsoluteHeading -= 2 * PI;
	while (realAbsoluteHeading < -PI) realAbsoluteHeading += 2 * PI;

	// this keeps the quad going at a constant speed given by desiredVelocity.
	updateVelocityActuators();

	// return values back to the hardware interface, as if theyd been measured.
	setAbsoluteHeading(realAbsoluteHeading + random() * headingAccuracy);

	// to mimic wheel encoder (perfectly accurate at 0m/s, and will always have direction right.
	// error gets worse as speed gets greater (hence, * realVelocity)
	if (realVelocity > 0) setVelocity(abs(realVelocity + random() * velocityAccuracy * realVelocity * 3));
	if (realVelocity < 0) setVelocity(-1 * abs(realVelocity + random() * velocityAccuracy * realVelocity * 3));
	if (realVelocity == 0) setVelocity(0);




	/**************************************
	updating 3 different ways we would know position, IMU, GPS, and mathematical model (kinematics)
	***************************************/

	// kinematics
	double kinDistanceTravelled = getVelocity() * time;
	double kinDistanceForward = 0;
	double kinDistanceRight = 0;
	double kinAngleTurned = 0;
	if (abs(getSteeringAngle()) < 0.01) {
		kinDistanceForward = kinDistanceTravelled;
	}
	else {
		double turnRadius = wheelBase / tan(-getSteeringAngle());
		kinAngleTurned = kinDistanceTravelled / turnRadius;
		kinDistanceForward = turnRadius * sin(angleTurned);
		kinDistanceRight = turnRadius - turnRadius * cos(kinAngleTurned);
	}
	kinematicPosition.x += kinDistanceForward * sin(kinematicHeading) + kinDistanceRight * cos(kinematicHeading);
	kinematicPosition.y += kinDistanceForward * cos(kinematicHeading) - kinDistanceRight * sin(kinematicHeading);
	kinematicHeading += kinAngleTurned;

	// GPS
	// gets updated once every second. has fairly constant error when moving, big fluctuations when still (not implemented).
	timeSinceLastGpsUpdate += time;
	if (timeSinceLastGpsUpdate >= 1.0) {
		double rand = random();
		Point newGpsPos = Point(gpsPosition.x + rand * gpsAccuracy, gpsPosition.y + rand * gpsAccuracy);
		while (newGpsPos.getDistanceTo(gpsPosition) > 0.4) {
			rand = random();
			newGpsPos = Point(gpsPosition.x + rand * gpsAccuracy, gpsPosition.y + rand * gpsAccuracy);
		}
		oldgpsPosition = gpsPosition;
		gpsPosition.x = realPosition.x + rand * gpsAccuracy;
		gpsPosition.y = realPosition.y + rand * gpsAccuracy;
		timeSinceLastGpsUpdate -= 1;
	}

	// Kalman filter:
	// update kalman kinematics
	double kalmanDistanceTravelled = getVelocity() * time;
	double kalmanDistanceForward = 0;
	double kalmanDistanceRight = 0;
	double kalmanAngleTurned = 0;
	if (abs(getSteeringAngle()) < 0.01) {
		kalmanDistanceForward = kalmanDistanceTravelled;
	}
	else {
		double turnRadius = wheelBase / tan(-getSteeringAngle());
		kalmanAngleTurned = kalmanDistanceTravelled / turnRadius;
		kalmanDistanceForward = turnRadius * sin(angleTurned);
		kalmanDistanceRight = turnRadius - turnRadius * cos(kalmanAngleTurned);
	}

	// update state prediction
	mu.put(0, 0, mu.get(0, 0) + kalmanDistanceForward * sin(mu.get(2, 0)) + kalmanDistanceRight * cos(mu.get(2, 0)));
	mu.put(1, 0, mu.get(1, 0) + kalmanDistanceForward * cos(mu.get(2, 0)) - kalmanDistanceRight * sin(mu.get(2, 0)));
	mu.put(2, 0, mu.get(2, 0) + kalmanAngleTurned);

	// the observation matrix
	Point gpsHeadingVector = gpsPosition - oldgpsPosition;
	double gpsAngle = PI / 2 - atan2(gpsHeadingVector.y, gpsHeadingVector.x);
	while (gpsAngle > PI) gpsAngle -= 2 * PI;
	while (gpsAngle < -PI) gpsAngle += 2 * PI;

	z.put(0, 0, gpsPosition.x);
	z.put(1, 0, gpsPosition.y);
	z.put(2, 0, gpsAngle);

	// update jacobian of g for our current position
	G.put(0, 2, kalmanDistanceForward * cos(mu.get(2, 0) - kalmanDistanceRight * sin(mu.get(2, 0))));
	G.put(1, 2, -kalmanDistanceForward * sin(mu.get(2, 0) - kalmanDistanceRight * cos(mu.get(2, 0))));

	// update R based on the time step, we are out by 0.5m every 20m
	// heading is out by 2 (s.d) degrees for every meter travelled
	R.put(0, 0, pow(kalmanDistanceTravelled * 0.025 / 2, 2));
	R.put(1, 1, pow(kalmanDistanceTravelled * 0.025 / 2, 2));
	R.put(2, 2, pow(kalmanDistanceTravelled * 2 * PI/180, 2));
	
	// update Q for the GPS, if we are moving, s.d = 3m at 0m/s, and s.d = 0.25m at 1.2m/s (full speed)
	double speedAccuracy = 3 - getVelocity() * 2.3;
	if (speedAccuracy < 0.25) speedAccuracy = 0.25;
	Q.put(0, 0, speedAccuracy);
	Q.put(1, 1, speedAccuracy);
	// heading, at speed = 0, gps is totally wrong, s.d = PI, at speed > 0.5m/s gps has s.d approx 10 degrees (0.17 rads)
	double steerAccuracy = PI - getVelocity() * 5.94;
	if (steerAccuracy < 0.17) steerAccuracy = 0.17;
	Q.put(2, 2, steerAccuracy);

	sigma = ((G * sigma) * G.getTranspose()) + R;

	if (oldgpsPosition.x != gpsPosition.x && oldgpsPosition.y != gpsPosition.y) {
		K = sigma * H.getTranspose() * (H * sigma * H.getTranspose() + Q).getInverse();
		mu = mu + K * (z - mu);
		sigma = (I - (K*H))*sigma;
	}

	setPosition(Point(mu.get(0, 0), mu.get(1, 0)));
	setAbsoluteHeading(realAbsoluteHeading);
}


void DummyHardware::setUpKalmanMatrices() {
	mu = Matrix<double>(3, 1);				// state space (x, y, theta)
	sigma = Matrix<double>(3, 3);				// uncertainty of motion
	g = Matrix<double>(3, 1);					// state update function
	K = Matrix<double>(3, 3);					// Kalman Gain
	G = Matrix<double>(3, 3);					// Jacobian of g
	R = Matrix<double>(3, 3);					// motion noise from g, variance matrix
	h = Matrix<double>(3, 1);					// observation function, h
	H = IdentityMatrix<double>(3, 3);			// Jacobian of h (in this case will remain as the identity matrix)
	z = Matrix<double>(3, 1);					// observation
	I = IdentityMatrix<double>(3, 3);			// identity matrix
	Q = Matrix<double>(3, 3);					// uncertainty of sensor observation

	mu.put(0, 0, realPosition.x);
	mu.put(1, 0, realPosition.y);
	mu.put(2, 0, realAbsoluteHeading);

	G = I;

}

Point DummyHardware::getKalmanPosition() {
	return Point(mu.get(0, 0), mu.get(1, 0));
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
void DummyHardware::setDesiredGear(Gear x) {
	if (gearTimer <= 0 && realGear != x)
		gearTimer = 0.5; // seconds per gear change
	
	desiredGear = x;
}

void DummyHardware::setDesiredVelocity(double x) {
	desiredVelocity = x;
}


// handles gear changes as well
void DummyHardware::updateVelocityActuators() {

	double throttlePercentageRequired = (abs(desiredVelocity) - 0.25) / 0.05;

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

Point DummyHardware::getKinematicPosition() {
	return kinematicPosition;
}
Point DummyHardware::getAccelerometerPosition() {
	return accelerometerPosition;
}
Point DummyHardware::getGPSPosition() {
	return gpsPosition;
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

void DummyHardware::emergencyStop() {

}