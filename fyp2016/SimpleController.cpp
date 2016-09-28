#include "SimpleController.h"


SimpleController::SimpleController()
{
	alive = true;
	enabled = false;
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
	ns = (SimpleNavigator*) nav;
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

		if (enabled) {
			updateDynamics();
		}

		start = end;

		//Cap at 1000Hz. no need for this to run any faster and chew cycles
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	return true;
}

void SimpleController::updateDynamics() {
	
	if (!hwi->getImuStabilised()) {
		hwi->setDesiredSteeringAngle(0);
		hwi->setDesiredVelocity(0);
		return;
	}

	Point quadPosition = hwi->getPosition();
	double quadHeading = hwi->getAbsoluteHeading();
	ns->updatePoint(quadPosition, (float)quadHeading, hwi->getVelocity());
	Point currentPoint = ns->getPoint();

	double angleToPathPoint = -1 * atan2(currentPoint.y - quadPosition.y, currentPoint.x - quadPosition.x) + PI / 2;
	angleToPathPoint = hwi->centreHeading(angleToPathPoint, 0);

	double alpha = angleToPathPoint - quadHeading;
	alpha = hwi->centreHeading(alpha, 0);
	double distance = quadPosition.getDistanceTo(currentPoint);
	double steerAngleReq = -atan(2 * hwi->wheelBase * sin(alpha) / distance);
	if (steerAngleReq > hwi->maxSteerAngle) steerAngleReq = hwi->maxSteerAngle;
	if (steerAngleReq < -hwi->maxSteerAngle) steerAngleReq = -hwi->maxSteerAngle;
	hwi->setDesiredSteeringAngle(steerAngleReq);

	double desiredVelocity;
	if (ns->isNextPoint()) {
		 desiredVelocity = distance * 1.8;
	}
	else {
		desiredVelocity = hwi->getVelocity() / 1.2;
	}
	
	if (desiredVelocity > hwi->cruiseVelocity) desiredVelocity = hwi->cruiseVelocity;
	if (abs(hwi->getSteeringAngle() - steerAngleReq) > 2 * PI / 180) desiredVelocity = 0;
	if (!ns->isConverging()) {
		desiredVelocity = 0;
	}

	// done by checking if point is in front or behid of quad bike now.
	/*if (ns->getIsForwards()) {
		hwi->setDesiredVelocity(desiredVelocity);
	}
	else {
		hwi->setDesiredVelocity(-desiredVelocity);
	}*/

	//if point is behind the quad bike we need to reverse to get tehre
	if (alpha > PI/2 || alpha < -PI/2) {
		desiredVelocity *= -1;
		
	}
	hwi->setDesiredVelocity(desiredVelocity);
}