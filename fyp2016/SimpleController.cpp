#include "SimpleController.h"


SimpleController::SimpleController()
{
	alive = true;
	navState = NAV_CRUISE;
	pathTravDir = 1;
	landmineDetected = false;
	currentPathPoint = 0;
}


SimpleController::~SimpleController()
{
	alive = false;
}


bool SimpleController::initialise(HardwareInterface* h, NavigationSystem* nav) {
	hwi = h;
	ns = nav;
	//hrt = HRTimer();
	start = std::chrono::high_resolution_clock::now();

	Log::d << "SimpleController initialised." << std::endl;

	/*************************************************************/
	/*
	delete updater;
	updater = NULL;
	alive = true;

	std::auto_ptr<Runnable> r(new ControlUpdaterRunnable(this));
	updater = new Thread(r);
	updater->start();
	*/

	updater = new std::thread(&SimpleController::updateLoop, this);

	Log::d << "Auto-controller sub-thread started." << std::endl;

	return true;
}

void SimpleController::setEnabled(bool b) {
	enabled = b;
	start = std::chrono::high_resolution_clock::now();
	//hrt.reset();
}

bool SimpleController::isEnabled() {
	return enabled;
}

bool SimpleController::isAlive() {
	return alive;
}

void SimpleController::landMineDetected() {
	landmineDetected = true;
	navState = NAV_LANDMINE_DETECTED;
}

bool SimpleController::updateLoop() {
	while (isAlive()) {
		// access the discrete time interval between consective exections
		//double seconds = hrt.getElapsedTimeSeconds();
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> seconds = end - start;

		// TODO(harry) : this needs to be time independent.
		// TODO(jono) : this is time independent...
		updateDynamics();

		//hrt.reset();
		start = end;

		//Cap at 1000Hz. no need for this to run any faster and chew cycles
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	return true;
}

void SimpleController::updateDynamics() {

	double desiredVel = 0;						// m/s
	double turnTolerance = 0.2;					// meters at which turn point is considered reached
	double lookAheadDistance = 1.2;				// meters
	double steerAngleTolerance = 8 * PI / 180;	// radians, if steer angle is out by more that this, quad will slow down (lowers desiredVelocity)

	if (currentPathPoint + pathTravDir >= ns->getPath().size() || currentPathPoint + pathTravDir < 0) {
		// next point doesnt exist
		desiredVel = 0;
		return;
	}

	// find angle between heading and to the next path point
	double angleToPathPoint = -1 * atan2(ns->getPath().at(currentPathPoint)->y - hwi->getPosition().y, ns->getPath().at(currentPathPoint)->x - hwi->getPosition().x) + PI / 2;
	if (angleToPathPoint > PI) angleToPathPoint -= 2 * PI;
	if (angleToPathPoint < -PI) angleToPathPoint += 2 * PI;
	double alpha = angleToPathPoint - hwi->getAbsoluteHeading();
	double distance = hwi->getPosition().getDistanceTo(*ns->getPath().at(currentPathPoint));
	double steerAngleReq = -atan(2 * hwi->wheelBase * sin(alpha) / distance);

	if (steerAngleReq > hwi->maxSteerAngle) steerAngleReq = hwi->maxSteerAngle;
	if (steerAngleReq < -hwi->maxSteerAngle) steerAngleReq = -hwi->maxSteerAngle;
	hwi->setDesiredSteeringAngle(steerAngleReq);


	if (distance > hwi->getPosition().getDistanceTo(*ns->getPath().at(currentPathPoint + pathTravDir))) {
		if (navState != NAV_LANDMINE_DETECTED) {
			// this means that we need to change direction when the quadbike reaches currentPathPoint (turn inbound?).
			navState = NAV_TURNINBOUND;
		}
	}

	// are we going forward (1) or backward (-1)
	int direction;
	if (alpha < -PI / 2 || alpha > PI / 2) {
		direction = -1;
	}
	else {
		direction = 1;
	}

	if (navState == NAV_TURNINBOUND) {
		// kinda bad because if quad overshoots it will keep going.
		desiredVel = direction * 2 * distance;
		if (abs(desiredVel) > hwi->cruiseVelocity)
			desiredVel = direction * hwi->cruiseVelocity;

		if (distance < turnTolerance) {
			navState = NAV_CRUISE;
			currentPathPoint += pathTravDir;
		}
	}
	else if (navState == NAV_CRUISE) {
		desiredVel = hwi->cruiseVelocity * direction;
		if (distance < lookAheadDistance) currentPathPoint += pathTravDir;
	}
	else if (navState == NAV_LANDMINE_DETECTED) {
		desiredVel = 0;
		if (hwi->getVelocity() == 0) {
			navState = NAV_CRUISE;
			pathTravDir = -1;

			// reset currentPathPoint to point near quadbike
			double initialDist = distance;
			double diff = 0;
			bool loop = true;
			while (loop) {
				currentPathPoint--;
				Point curPos = hwi->getPosition();
				distance = curPos.getDistanceTo(*ns->getPath().at(currentPathPoint));
				if (initialDist - distance < diff) {
					loop = false;
				}
				else {
					diff = initialDist - distance;
				}
			}
		}
	}
	
	if (abs(hwi->getSteeringAngle() - steerAngleReq) > steerAngleTolerance) {
		//desiredVel = 0;
	}

	hwi->setDesiredVelocity(desiredVel);

}