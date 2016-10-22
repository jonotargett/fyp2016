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
double DummyHardware::random(double low, double high) {
	return ((double)rand() / (RAND_MAX) * (high - low)) + low;
}


bool DummyHardware::initialise() {

	velocityChangeRate = 0.6;		// metres/second^2
	frictionalDecayRate = 120;		// %/second??
	brakingAcceleration = 20;		// m/s/s at 100% brake. interpolate inbetween

	headingAccuracy = 0;			// radians of spread each side of real value (this wont be a thing, will come from kalman filter)
	velocityAccuracy = 0.025;		// m/s of spread each side of real value @ full speed (0.5m error over 20m)
	steeringAccuracy = 1 * PI/180;	// radians of spread each side of real value at full lock
	brakeAccuracy = 0;				// percent of spread each side of real value
	throttleAccuracy = 0;			// percent of spread each side of real value
	gpsAccuracy = 2.5;				// meters spread each side of real value
	maxGpsMove = 0.5;				// max distance the gps can move on each update (from previous position)
	imuFloat = 1/360;				// degrees of inaccuracy per degree

	srand((unsigned int)time(NULL));
	//hrt = HRTimer();
	startTime = std::chrono::high_resolution_clock::now();

	// we KNOW the starting position and heading of the quad bike
	realPosition = Point(0, 0);
	realAbsoluteHeading = 0.0 * PI / 180;
	
	realVelocity = 0.0;
	realSteeringAngle = 0.0;
	realThrottlePercentage = 0.0;
	realGear = GEAR_NEUTRAL;
	
	desiredSteeringAngle = 0.0;
	desiredVelocity = 0.0;

	// initialising sensor information
	kinematicPosition = realPosition;
	kinematicHeading = realAbsoluteHeading;
	imuHeading = random(-PI, PI);		// we have no idea what the first value of the heading is going to be!
	imuFloat += random(-imuFloat / 10, imuFloat / 10);
	Log::i << "imuFloat: " << imuFloat << endl;

	oldPositionAtGpsUpdate = realPosition;
	setPosition(realPosition);
	setSteeringAngle(realSteeringAngle + random() * steeringAccuracy);
	setBrakePercentage(realBrakePercentage + random() * brakeAccuracy);
	setThrottlePercentage(realThrottlePercentage + random() * throttleAccuracy);
	setGear(realGear);
	setImuHeading(imuHeading);
	setPosition(realPosition);
	setAbsoluteHeading(realAbsoluteHeading);
	setGpsPosition(realPosition);

	setImuInitialHeading(imuHeading);

	timeSinceLastGpsUpdate = 0.0;

	resetKalmanState(realPosition, realAbsoluteHeading);
	

	// return true once everything is initialised.
	// seeing as there is no actual hardware here,
	// just return true. oh yeah
	Log::d << "DummyHardware initialised." << std::endl;

	start();

	//error visualisation, see update()
	//myfile.open("data.txt");
	//myfile << "time realx realy realh kalx kaly kalh gpsx gpsy gpsh kinx kiny kinh imuh" << endl;

	return true;
}

void DummyHardware::update(double time) { // gets refreshed at 50Hz as defined by REFRESH_RATE

	// was for writing things to a file for error visualisation
	/*Point gpsHeadingVector = getGpsPosition() - gpsPrevPosition;
	double gpsHeading;
	gpsHeading = PI / 2 - atan2(gpsHeadingVector.y, gpsHeadingVector.x); // converts to our origin and direction
	gpsHeading = centreHeading(gpsHeading, 0);

	myfile << timeasdfa << " ";
	myfile << realPosition.x << " " << realPosition.y << " " << centreHeading(realAbsoluteHeading, 0) * 180/PI << " ";
	myfile << getKalmanPosition().x << " " << getKalmanPosition().y << " " << getKalmanHeading() * 180/PI << " ";
	myfile << getGpsPosition().x << " " << getGpsPosition().y << " " << gpsHeading * 180 / PI << " ";
	myfile << getKinematicPosition().x << " " << getKinematicPosition().y << " " << getKinematicHeading() * 180/PI << " ";
	myfile << getImuHeading() * 180 / PI << endl;
	timeasdfa += time;*/



	
	// actuator logic (fake a slow merge towards desired position)
	updateActuators(time);
	
	// ************************************************************************************************
	// mimicking what the real quad bike would be doing:
	// ************************************************************************************************
	
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

	// updating real position
	Point oldPosition = realPosition;
	realPosition.x += distanceForward * sin(realAbsoluteHeading) + distanceRight * cos(realAbsoluteHeading);
	realPosition.y += distanceForward * cos(realAbsoluteHeading) - distanceRight * sin(realAbsoluteHeading);
	realAbsoluteHeading += angleTurned;

	while (realAbsoluteHeading > PI) realAbsoluteHeading -= 2 * PI;
	while (realAbsoluteHeading < -PI) realAbsoluteHeading += 2 * PI;

	// ****************************************************************************************************
	// control function to keep the quad going at a constant velocity given desiredVelocity
	// **************************************************************************************************
	// this probably belongs in HardwareInterface but whatever.
	// TODO() move to the HardwareInterface
	updateVelocityActuators();


	/**************************************
	updating ways we would know position, IMU, GPS
	***************************************/
	// GPS
	// gets updated once every second. has fairly constant error when moving, big fluctuations when still (not implemented).
	// max movement of gps each second is as listed in maxGpsMove;
	// new gps point needs to be within 2.5 meters of new real position, and within maxGpsMove of old gps coordinate (after being updated)
	Point gpsPosition = getGpsPosition(); // need to set to getGpsPosition otherwise if it doesnt update here it will setGpsPosition to (0, 0) later on
	timeSinceLastGpsUpdate += time;
	if (timeSinceLastGpsUpdate >= 1.0) {
		Point realMoveVector = realPosition - oldPositionAtGpsUpdate;

		double randx = pow(random(), 1);
		double randy = pow(random(), 1);
		Point newGpsPos = Point(realPosition.x + randx * gpsAccuracy, realPosition.y + randy * gpsAccuracy);

			while (newGpsPos.getDistanceTo(oldPositionAtGpsUpdate + realMoveVector) > maxGpsMove) {
				randx = pow(random(), 1);
				randy = pow(random(), 1);
				newGpsPos = Point(realPosition.x + randx * gpsAccuracy, realPosition.y + randy * gpsAccuracy);
			}
		oldPositionAtGpsUpdate = realPosition;

		gpsPosition.x = realPosition.x + randx * gpsAccuracy;
		gpsPosition.y = realPosition.y + randy * gpsAccuracy;

		timeSinceLastGpsUpdate -= 1;
	}

	// imu
	// imu heading is pretty accurate, for our purposes here it adds imuFloat (error) for every radian traversed.
	imuHeading += angleTurned + abs(imuFloat * angleTurned);
	if (imuHeading > PI) imuHeading -= 2 * PI;
	if (imuHeading < -PI) imuHeading += 2 * PI;


	// kinematics (debug purposes only) simply draw this point ot the screen
	// update kalman quad bike kinematics
	double kinDistanceTravelled = getVelocity() * time;
	double kinDistanceForward = 0;
	double kinDistanceRight = 0;
	double kinAngleTurned = 0;
	if (abs(getSteeringAngle()) < 0.01) {
		kinDistanceForward = kinDistanceTravelled;
	}
	else {
		double kinTurnRadius = wheelBase / tan(-getSteeringAngle());
		kinAngleTurned = kinDistanceTravelled / kinTurnRadius;
		kinDistanceForward = kinTurnRadius * sin(kinAngleTurned);
		kinDistanceRight = kinTurnRadius - kinTurnRadius * cos(kinAngleTurned);
	}

	kinematicHeading += kinAngleTurned;

	kinematicPosition.x += kinDistanceForward * sin(kinematicHeading) + kinDistanceRight * cos(kinematicHeading);
	kinematicPosition.y += kinDistanceForward * cos(kinematicHeading) - kinDistanceRight * sin(kinematicHeading);
	

	// ************************************************************************************************
	// return values back to the hardeware interface, as if they had been measured.
	// *************************************************************************************************
	setSteeringAngle(realSteeringAngle + steeringAccuracy * realSteeringAngle/(24*PI/180));
	setBrakePercentage(realBrakePercentage + random() * brakeAccuracy);
	setThrottlePercentage(realThrottlePercentage + random() * throttleAccuracy);
	setGear(realGear);
	setImuHeading(imuHeading);
	//setPosition(realPosition); // this is done in the hardwareinterface class
	setAbsoluteHeading(realAbsoluteHeading);
	setGpsPosition(gpsPosition);

	// to mimic wheel encoder (perfectly accurate at 0m/s, and will always have direction right.
	// error gets worse as speed gets greater (hence, * realVelocity)
	if (realVelocity > 0) setVelocity(abs(realVelocity + random(0.5, 1) * velocityAccuracy * realVelocity / 1.2));
	if (realVelocity < 0) setVelocity(-1 * abs(realVelocity - random(0.5, 1) * velocityAccuracy * realVelocity / 1.2));
	if (realVelocity == 0) setVelocity(0);

	updateHardware(time);
}

double DummyHardware::getKinematicHeading() {
	return kinematicHeading;
}
Point DummyHardware::getKinematicPosition() {
	return kinematicPosition;
}

void DummyHardware::updateActuators(double time) {
	// all actuator update logic goes here (not needed for real quad bike)
	// fake a slow merge towards actuator positions

	double steerChangeRate = 0.5;			// radians/second
	double gearChangeRate = 1;				// second / gear change
	double throttleChangeRate = 200;		// percentChange/second
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
	if (x > maxSteerAngle) x = maxSteerAngle;
	if (x < -maxSteerAngle) x = -maxSteerAngle;
	
	desiredSteeringAngle = x;
}
void DummyHardware::setDesiredThrottlePercentage(double x) {
	if (x > 100) x = 100;
	if (x < 0) x = 0;
	if (x > 0) setDesiredBrake(0);
	desiredThrottlePercentage = x;
}
void DummyHardware::setDesiredBrake(double x) {
	if (x > 100) x = 100;
	if (x < 0) x = 0;
	if (x > 0) setDesiredThrottlePercentage(0);
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

	/*if (desiredVelocity > 0) {
		if (getGear() != GEAR_FORWARD)	setDesiredGear(GEAR_FORWARD);
		setDesiredThrottlePercentage(20);
	}
	if (desiredVelocity < 0) {
		if (getGear() != GEAR_REVERSE)	setDesiredGear(GEAR_REVERSE);
		setDesiredThrottlePercentage(20);
	}
	if (desiredVelocity == 0) {
		if (getGear() != GEAR_NEUTRAL)	setDesiredGear(GEAR_NEUTRAL);
		setDesiredThrottlePercentage(0);
	}*/

	double throttlePercentageRequired = (abs(desiredVelocity) - 0.25) / 0.05;
	double testThrottle = (1.4 - 0.25) / 0.05;

	if (desiredVelocity == 0) {
		setDesiredThrottlePercentage(0);
		setDesiredGear(GEAR_NEUTRAL);
		setDesiredBrake(50);
	}
	else if (desiredVelocity > 0) {
		// if we are travelling in the wrong direction
		if (getVelocity() < 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrake(50);
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
				setDesiredBrake(50);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large 
		// difference in actual speed and desired speed
		else {
			setDesiredThrottlePercentage(testThrottle);
		}
	}
	else if (desiredVelocity < 0) {
		// if we are travelling in the wrong direction
		if (getVelocity()  > 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrake(50);
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
				setDesiredBrake(50);
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
			setDesiredThrottlePercentage(testThrottle);
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

Gear DummyHardware::getRealGear() {
	return realGear;
}

double DummyHardware::getRealBrakePercentage() {
	return realBrakePercentage;
}

void DummyHardware::emergencyStop() {

}