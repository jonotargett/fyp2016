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
	gpsPositionLock = false;
	imuHeadingLock = false;

	imuStable = false;
	
	position = Point();
	gpsPrevPosition = Point();
	kinematicAlteredGps = Point();

	mu = Matrix<double>(3, 1);
	sigma = Matrix<double>(3, 3);

	alive = true;
}


HardwareInterface::~HardwareInterface()
{
	// signal to the child threads that were heading for shutdown
	alive = false;
}

bool HardwareInterface::getImuStabilised() {
	return imuStable;
}

void HardwareInterface::updateHardware(double time) {
	//need to make sure imu has stabilised
	/*if (!imuStable) {
		if (imuInitialHeading - imuHeading == 0) {
			imuStable = true;
		}
		imuInitialHeading = imuHeading;
		Log::d << "Waiting for IMU to stabilise..." << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		return;
	}*/
	updateKalmanFilter(time);
}

void HardwareInterface::setImuInitialHeading(double x) {
	imuInitialHeading = x;
}

void HardwareInterface::updateKalmanFilter(double time) {
	//kalman filter matrices:
	// prediction step:
	Matrix<double> g = Matrix<double>(3, 1);					// state update function
	Matrix<double> G = Matrix<double>(3, 3);					// Jacobian of g
	Matrix<double> R = Matrix<double>(3, 3);					// motion covariance from g
	Matrix<double> I = IdentityMatrix<double>(3, 3);			// identity matrix

	Matrix<double> z;											// observation
	Matrix<double> Q;											// uncertainty of sensor observation
	Matrix<double> K;											// Kalman Gain
	Matrix<double> H;

	Point posPrior = Point(mu.get(0, 0), mu.get(1, 0));
	double headingPrior = mu.get(2, 0);

	// update kalman quad bike kinematics
	double kinDistanceTravelled = getVelocity() * time;
	double kinDistanceForward = 0;
	double kinDistanceRight = 0;
	double kinAngleTurned = 0;
	if (abs(getSteeringAngle()) < 0.001) {
		kinDistanceForward = kinDistanceTravelled;
	}
	else {
		double turnRadius = wheelBase / tan(-getSteeringAngle());
		kinAngleTurned = kinDistanceTravelled / turnRadius;
		kinDistanceForward = turnRadius * sin(kinAngleTurned);
		kinDistanceRight = turnRadius - turnRadius * cos(kinAngleTurned);
	}

	double globalDeltaX = kinDistanceForward * sin(headingPrior) + kinDistanceRight * cos(headingPrior);
	double globalDeltaY = kinDistanceForward * cos(headingPrior) - kinDistanceRight * sin(headingPrior);

	kinematicAlteredGps = (Point(kinematicAlteredGps.x + globalDeltaX, kinematicAlteredGps.y + globalDeltaY));

	// update state prediction
	mu.put(0, 0, posPrior.x + globalDeltaX);
	mu.put(1, 0, posPrior.y + globalDeltaY);
	mu.put(2, 0, headingPrior + kinAngleTurned);

	// update jacobian of g for our current position
	G.put(0, 2, kinDistanceForward * cos(headingPrior) - kinDistanceRight * sin(headingPrior));
	G.put(1, 2, -kinDistanceForward * sin(headingPrior) - kinDistanceRight * cos(headingPrior));

	// update R based on the time step, we are out by 0.5m every 20m
	// heading is out by 5 (s.d) degrees for every meter travelled
	R.put(0, 0, pow(kinDistanceTravelled * 0.025 *8, 2));
	R.put(1, 1, pow(kinDistanceTravelled * 0.025 * 8, 2));
	R.put(2, 2, pow(kinDistanceTravelled * 5 * PI/180, 2));

	sigma = ((G * sigma) * G.getTranspose()) + R;

	//////////////////////////////////
	////// GPS observations://////////
	//////////////////////////////////

	//////// POSITION/////////////////
	/*z = Matrix<double>(2, 1);					// observation
	Q = Matrix<double>(2, 2);					// uncertainty of sensor observation
	K = Matrix<double>(3, 2);					// Kalman Gain
	H = Matrix<double>(2, 3);					// Jacobian of h

	Matrix<double> muPart = Matrix<double>(2, 1);
	muPart.put(0, 0, mu.get(0, 0));
	muPart.put(1, 0, mu.get(1, 0));
	z.put(0, 0, kinematicAlteredGps.x);
	z.put(1, 0, kinematicAlteredGps.y);
	H.put(0, 0, 1);
	H.put(1, 1, 1);
	
	// update Q for the GPS, position s.d = 2m at 0m/s, and s.d = 0.2m at 1.2m/s (full speed).
	// heading, at speed = 0, gps is totally wrong, s.d = infinity, at speed = 0.5m/s gps has s.d approx 
	// 10 degrees (0.17 rads), at speed = 1.2m/s gps has s.d approx 3 degrees
	double positionAccuracy = 1 - getVelocity() * 0.8;
	if (positionAccuracy < 0.1) positionAccuracy = 0.1;
	Q.put(0, 0, pow(positionAccuracy, 2));
	Q.put(1, 1, pow(positionAccuracy, 2));

	K = sigma * H.getTranspose() * (H * sigma * H.getTranspose() + Q).getInverse();
	mu = mu + K * (z - muPart);
	sigma = (I - (K*H))*sigma;


	/////////Heading///////////////
	z = Matrix<double>(1, 1);					// observation
	Q = Matrix<double>(1, 1);					// uncertainty of sensor observation
	K = Matrix<double>(3, 1);					// Kalman Gain
	H = Matrix<double>(1, 3);					// Jacobian of h

	Point gpsHeadingVector = kinematicAlteredGps - gpsPrevPosition;
	if (gpsHeadingVector.getLength() != 0) {
		gpsHeading = PI / 2 - atan2(gpsHeadingVector.y, gpsHeadingVector.x); // converts to our origin and direction
		gpsHeading = centreHeading(gpsHeading, 0);
	}
	double headingAccuracy = 1 / (gpsHeadingVector.getLength() / 2);
	if (getVelocity() < 0.8 || getSteeringAngle() > 5*PI/180) {
		headingAccuracy = INFINITY;
	}
	z.put(0, 0, centreHeading(gpsHeading, headingPrior));
	Q.put(0, 0, pow(5 * (PI / 180) * headingAccuracy, 2));
	H.put(0, 2, 1);

	K = (sigma * H.getTranspose()) * (H * sigma * H.getTranspose() + Q).getInverse();
	mu = mu + K * (z.get(0, 0) - mu.get(2, 0));
	sigma = (I - (K*H))*sigma;
	
	/////////////////////////////////
	////// IMU observations://///////
	//////////////////////////////////
	z = Matrix<double>(1, 1);					// observation
	Q = Matrix<double>(1, 1);					// uncertainty of sensor observation
	K = Matrix<double>(3, 1);					// Kalman Gain
	H = Matrix<double>(1, 3);					// Jacobian of h

	double deltaHeading = imuHeading - centreHeading(imuInitialHeading, imuHeading);

	z.put(0, 0, headingPrior + deltaHeading);
	Q.put(0, 0, pow(deltaHeading * 3 * PI / 180, 2));
	H.put(0, 2, 1);

	K = (sigma * H.getTranspose()) * (H * sigma * H.getTranspose() + Q).getInverse();
	mu = mu + K * (z.get(0,0) - mu.get(2, 0));
	sigma = (I - (K*H))*sigma;

	imuInitialHeading = imuHeading;*/


	/////////////////////////////////
	////// updating position /////////
	//////////////////////////////////
	// set the actual position and heading to what we calculate:
	mu.put(2, 0, centreHeading(mu.get(2, 0), 0));
	setPosition(Point(mu.get(0, 0), mu.get(1, 0)));
	setAbsoluteHeading(mu.get(2, 0));

}

double HardwareInterface::centreHeading(double h, double centre) {
	double heading = h;
	while (heading > centre + PI) heading -= 2 * PI;
	while (heading < centre - PI) heading += 2 * PI;
	return heading;
}

void HardwareInterface::resetKalmanState(Point position, double heading) {
	mu.put(0, 0, position.x);
	mu.put(1, 0, position.y);
	mu.put(2, 0, heading);
}

double HardwareInterface::getGpsHeading() {
	return gpsHeading;
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

void HardwareInterface::setManualControl(bool b) {
	manualControl = b;
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
	if (getGear() == GEAR_NEUTRAL && getThrottlePercentage() == 0) {
		x = 0;
	}

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
	// if gps has actually updated
	if (gpsPosition.x != x.x || gpsPosition.y != x.y) {
		gpsPrevPosition = gpsPosition;
		kinematicAlteredGps = x;
	}
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