#include "SimpleController.h"


SimpleController::SimpleController()
{
	alive = true;
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
	navState = "landmineDetected";
}

bool SimpleController::updateLoop() {
	while (isAlive()) {
		// access the discrete time interval between consective exections
		//double seconds = hrt.getElapsedTimeSeconds();
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> seconds = end - start;

		updateDynamics();

		//hrt.reset();
		start = end;

		//Cap at 1000Hz. no need for this to run any faster and chew cycles
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	return true;
}

void SimpleController::updateDynamics() {

	double desiredVel = 0;

	if (currentPathPoint + pathTravDir >= ns->getPath().size() || currentPathPoint + pathTravDir < 0) {
		// next point doesnt exist
		return;
	}

	// find angle between heading and to the next path point
	double angleToPathPoint = -1 * atan2(ns->getPath().at(currentPathPoint)->y - hwi->getPosition().y, ns->getPath().at(currentPathPoint)->x - hwi->getPosition().x) + 3.14159265 / 2;
	if (angleToPathPoint > 3.141593) angleToPathPoint -= 2 * 3.141593;
	if (angleToPathPoint < -3.141593) angleToPathPoint += 2 * 3.141593;
	double alpha = angleToPathPoint - hwi->getAbsoluteHeading();
	double distance = hwi->getPosition().getDistanceTo(*ns->getPath().at(currentPathPoint));
	double steerAngleReq = -atan(2 * hwi->wheelBase * sin(alpha) / distance);

	if (steerAngleReq > hwi->maxSteerAngle) steerAngleReq = hwi->maxSteerAngle;
	if (steerAngleReq < -hwi->maxSteerAngle) steerAngleReq = -hwi->maxSteerAngle;
	hwi->setDesiredSteeringAngle(steerAngleReq);


	if (distance > hwi->getPosition().getDistanceTo(*ns->getPath().at(currentPathPoint + pathTravDir))) {
		if (navState != "landmineDetected") {
			// this means that we need to change direction when the quadbike reaches currentPathPoint (turn inbound?).
			navState = "turnInbound";
		}
	}

	// are we going forward (1) or backward (-1)
	int direction;
	if (alpha < -3.1416 / 2 || alpha > 3.1416 / 2) {
		direction = -1;
	}
	else {
		direction = 1;
	}

	if (navState == "turnInbound") {
		// kinda bad because if quad overshoots it will keep going.
		desiredVel = direction * 2 * distance;
		if (abs(desiredVel) > hwi->cruiseVelocity)
			desiredVel = direction * hwi->cruiseVelocity;

		if (distance < 0.2) {
			navState = "cruise";
			currentPathPoint += pathTravDir;
		}
	}
	else if (navState == "cruise") {
		desiredVel = hwi->cruiseVelocity * direction;
		if (distance < 1.2) currentPathPoint += pathTravDir;
	}
	else if (navState == "landmineDetected") {
		desiredVel = 0;
		if (hwi->getVelocity() == 0) {
			navState = "cruise";
			pathTravDir = -1;

			// reset currentPathPoint to point near quadbike
			double initialDist = distance;
			double diff = 0;
			bool loop = true;
			while (loop) {
				currentPathPoint--;
				distance = hwi->getPosition().getDistanceTo(*ns->getPath().at(currentPathPoint));
				if (initialDist - distance < diff) {
					loop = false;
				}
				else {
					diff = initialDist - distance;
				}
			}
		}
	}
	
	if (abs(hwi->getSteeringAngle() - steerAngleReq) > 3 * 3.1416 / 180) {
		desiredVel = 0;
	}

	hwi->setDesiredVelocity(desiredVel);

}