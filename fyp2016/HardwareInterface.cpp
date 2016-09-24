#include "HardwareInterface.h"


/* 
Base class constructor will be called by default
in inherited classes. All the stuff common to all hardwareInterface
object types should be instantiated here
*/
HardwareInterface::HardwareInterface()
{
	// instantiate all the variables
	positionLock = false;
	position = Point();

	gpsPositionLock = false;
	imuHeadingLock = false;

	mu = Matrix<double>(3, 1);
	sigma = Matrix<double>(3, 3);

	alive = true;
}


HardwareInterface::~HardwareInterface()
{
	// signal to the child threads that were heading for shutdown
	alive = false;
}

void HardwareInterface::updateKalmanFilter(double time) {

	//kalman filter matrices:
	// prediction step:
	Matrix<double> g = Matrix<double>(3, 1);					// state update function

	// observation step:
	Matrix<double> K = Matrix<double>(3, 3);					// Kalman Gain
	Matrix<double> G = Matrix<double>(3, 3);					// Jacobian of g
	Matrix<double> R = Matrix<double>(3, 3);					// motion covariance from g
	Matrix<double> h = Matrix<double>(3, 1);					// observation function, h
	Matrix<double> H = Matrix<double>(3, 3);					// Jacobian of h
	Matrix<double> z = Matrix<double>(3, 1);					// observation
	Matrix<double> I = IdentityMatrix<double>(3, 3);			// identity matrix
	Matrix<double> Q = Matrix<double>(3, 3);					// uncertainty of sensor observation


	// update kalman quad bike kinematics
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
		kinDistanceForward = turnRadius * sin(kinAngleTurned);
		kinDistanceRight = turnRadius - turnRadius * cos(kinAngleTurned);
	}	

	// update state prediction
	mu.put(0, 0, mu.get(0, 0) + kinDistanceForward * sin(mu.get(2, 0)) + kinDistanceRight * cos(mu.get(2, 0)));
	mu.put(1, 0, mu.get(1, 0) + kinDistanceForward * cos(mu.get(2, 0)) - kinDistanceRight * sin(mu.get(2, 0)));
	mu.put(2, 0, mu.get(2, 0) + kinAngleTurned);

	// update jacobian of g for our current position
	G.put(0, 2, kinDistanceForward * cos(mu.get(2, 0) - kinDistanceRight * sin(mu.get(2, 0))));
	G.put(1, 2, -kinDistanceForward * sin(mu.get(2, 0) - kinDistanceRight * cos(mu.get(2, 0))));

	// update R based on the time step, we are out by 0.5m every 20m
	// heading is out by 5 (s.d) degrees for every meter travelled
	R.put(0, 0, pow(kinDistanceTravelled * 0.025, 2));
	R.put(1, 1, pow(kinDistanceTravelled * 0.025, 2));
	R.put(2, 2, pow(kinDistanceTravelled * 5 * PI/180, 2));

	sigma = ((G * sigma) * G.getTranspose()) + R;


	// if we have a new GPS coordinate put it through the kalman filter
	/*if (gpsUpdated) {
		// the observation matrix
		Point gpsHeadingVector = gpsPosition - oldKalmanPositionAtLastGPS;
		double gpsAngle = PI / 2 - atan2(gpsHeadingVector.y, gpsHeadingVector.x);
		while (gpsAngle > PI) gpsAngle -= 2 * PI;
		while (gpsAngle < -PI) gpsAngle += 2 * PI;
		z.put(0, 0, gpsPosition.x);
		z.put(1, 0, gpsPosition.y);
		z.put(2, 0, gpsAngle);

		// update Q for the GPS, if we are moving, s.d = 3m at 0m/s, and s.d = 0.25m at 1.2m/s (full speed)
		double positionAccuracy = 3 - getVelocity() * 2.3;
		if (positionAccuracy < 0.25) positionAccuracy = 0.25;
		Q.put(0, 0, 0.25);
		Q.put(1, 1, 0.25);
		// heading, at speed = 0, gps is totally wrong, s.d = PI, at speed = 0.5m/s gps has s.d approx 10 degrees (0.17 rads), at speed = 1.3m/s gps has s.d approx 3 degrees
		//double headingAccuracy = exp(-4 * getVelocity()) + 3 * PI/180;
		double headingAccuracy = 20;
		Q.put(2, 2, headingAccuracy);

		K = sigma * H.getTranspose() * (H * sigma * H.getTranspose() + Q).getInverse();
		mu = mu + K * (z - mu);
		sigma = (I - (K*H))*sigma;

		oldKalmanPositionAtLastGPS = Point(mu.get(0, 0), mu.get(1, 0));
		gpsUpdated = false;

	}*/
}

void HardwareInterface::resetKalmanState(Point position, double heading) {
	mu.put(0, 0, position.x);
	mu.put(1, 0, position.y);
	mu.put(2, 0, heading);
}

Point HardwareInterface::getKalmanPosition() {
	return Point(mu.get(0, 0), mu.get(1, 0));
}
double HardwareInterface::getKalmanHeading() {
	return mu.get(2, 0);
}

bool HardwareInterface::isAlive() {
	return alive;
}

bool HardwareInterface::start() {
	// start the updater thread
	/*
	delete updater;
	updater = NULL;
	alive = true;

	std::auto_ptr<Runnable> r(new UpdaterRunnable(this));
	updater = new Thread(r);
	updater->start();
	*/

	updater = new std::thread(&HardwareInterface::updateLoop, this);

	Log::d << "Hardware communication sub-thread started." << std::endl;

	// return true if everything worked
	return true;
}

void HardwareInterface::stop() {
	// kills the subthread
	alive = false;
}

/*
void* UpdaterRunnable::run() {

	bool success = true;

	while (hwi->isAlive()) {
		// no point in doing this as fast as humanly possible - the other end wont keep up
		// CAP AT 1000Hz
		SDL_Delay(1);

		success = hwi->updateLoop();

		if (!success) {
			return (void*)false;
		}
	}

	return (void*)true;
}
*/


void HardwareInterface::setDesiredVelocity(double x) {}
void HardwareInterface::setDesiredSteeringAngle(double x) {}
void HardwareInterface::setDesiredThrottlePercentage(double x) {}
void HardwareInterface::setDesiredBrakePercentage(double x) {}
void HardwareInterface::setDesiredGear(Gear x) {}
void HardwareInterface::updateVelocityActuators() {}


/*********************************************************************************/
// GETTERS AND SETTERS FOR HW VARIABLES FOR THREAD SAFETY
/*********************************************************************************/

Point HardwareInterface::getPosition() {
	while (positionLock) {
		/*
		threadwait. The update thread will lock variables as it
		updates them to prevent r/w issues. This allows the updater
		to run continuously and provide the most up to date information
		possible.
		*/ 
	}

	return position;
}

void HardwareInterface::setPosition(Point p) {
	while (positionLock) {
		/*
		threadwait. The update thread will lock variables as it
		updates them to prevent r/w issues. This allows the updater
		to run continuously and provide the most up to date information
		possible.
		*/
	}

	/*
	Trigger a lock over this value - preventing another object from accessing it.
	Threadsafety and whatnot. Then update, and release.
	*/
	positionLock = true;
	position = p;
	positionLock = false;
}


double HardwareInterface::getAbsoluteHeading() {
	while (absoluteHeadingLock) {
		//wait
	}
	return absoluteHeading;
}
void HardwareInterface::setAbsoluteHeading(double x) {
	while (absoluteHeadingLock) {
		//wait
	}
	absoluteHeadingLock = true;
	absoluteHeading = x;
	absoluteHeadingLock = false;
}


double HardwareInterface::getVelocity() {
	while (velocityLock) {
		//wait
	}
	return velocity;
}
void HardwareInterface::setVelocity(double x) {
	while (velocityLock) {
		//wait
	}
	velocityLock = true;
	velocity = x;
	velocityLock = false;
}

double HardwareInterface::getSteeringAngle() {
	while (steeringAngleLock) {
		//wait
	}
	return steeringAngle;
}
void HardwareInterface::setSteeringAngle(double x) {
	while (steeringAngleLock) {
		//wait
	}
	steeringAngleLock = true;
	steeringAngle = x;
	steeringAngleLock = false;
}

double HardwareInterface::getBrakePercentage() {
	while (brakeLock) {
		//wait
	}
	return brakePercentage;
}
void HardwareInterface::setBrakePercentage(double x) {
	while (brakeLock) {
		//wait
	}
	brakeLock = true;
	brakePercentage = x;
	brakeLock = false;
}


double HardwareInterface::getThrottlePercentage() {
	while (throttlePercentageLock) {
		//wait
	}
	return throttlePercentage;
}
void HardwareInterface::setThrottlePercentage(double x) {
	while (throttlePercentageLock) {
		//wait
	}
	throttlePercentageLock = true;
	throttlePercentage = x;
	throttlePercentageLock = false;
}

Gear HardwareInterface::getGear() {
	while (gearLock) {
		//wait
	}
	return gear;
}
void HardwareInterface::setGear(Gear x) {
	while (gearLock) {
		//wait
	}
	gearLock = true;
	gear = x;
	gearLock = false;
}

Point HardwareInterface::getGpsPosition() {
	while (gpsPositionLock) {
		//wait
	}
	return gpsPosition;
}
void HardwareInterface::setGpsPosition(Point x) {
	while (gpsPositionLock) {
		//wait
	}
	gpsPositionLock = true;
	gpsPosition = x;
	gpsPositionLock = false;
}

double HardwareInterface::getImuHeading() {
	while (imuHeadingLock) {
		//wait
	}
	return imuHeading;
}
void HardwareInterface::setImuHeading(double x) {
	while (imuHeadingLock) {
		//wait
	}
	imuHeadingLock = true;
	imuHeading = x;
	imuHeadingLock = false;
}