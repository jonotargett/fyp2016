#include "SimpleController.h"



SimpleController::SimpleController()
{
	alive = true;
}


SimpleController::~SimpleController()
{
	alive = false;
}


bool SimpleController::initialise(HardwareInterface* h) {
	hwi = h;

	hrt = HRTimer();
	std::cout << "timer generated" << std::endl;

	/*************************************************************/

	delete updater;
	updater = NULL;
	alive = true;

	std::auto_ptr<Runnable> r(new ControlUpdaterRunnable(this));
	updater = new Thread(r);
	updater->start();

	std::cout << "Auto-controller sub-thread started." << std::endl;

	return true;
}

void SimpleController::setEnabled(bool b) {
	enabled = b;
	hrt.reset();
}

bool SimpleController::isEnabled() {
	return enabled;
}

bool SimpleController::isAlive() {
	return alive;
}


bool SimpleController::setInputs(double relativeHeading, double distance) {

	// do something to sanitise these inputs?

	heading = relativeHeading;
	dist = distance;

	return true;
}

bool SimpleController::updateLoop() {
	// access the discrete time interval between consective exections
	double seconds = hrt.getElapsedTimeSeconds();

	if (enabled) {
		// implement discretised time PID controller in here
	}

	hrt.reset();
	return true;
}


void* ControlUpdaterRunnable::run() {

	bool success = true;

	while (controller->isAlive()) {

		success = controller->updateLoop();

		if (!success) {
			return (void*)false;
		}
	}

	return (void*)true;
}