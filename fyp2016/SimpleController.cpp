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

	//hrt = HRTimer();
	start = std::chrono::high_resolution_clock::now();

	Log::i << "SimpleController initialised." << std::endl;

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

	Log::i << "Auto-controller sub-thread started." << std::endl;

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


bool SimpleController::setInputs(double relativeHeading, double distance) {

	// do something to sanitise these inputs?

	heading = relativeHeading;
	dist = distance;

	return true;
}

bool SimpleController::updateLoop() {
	while (isAlive()) {
		// access the discrete time interval between consective exections
		//double seconds = hrt.getElapsedTimeSeconds();
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> seconds = end - start;

		if (enabled) {
			// implement discretised time PID controller in here

			if (dist > 25) {
				hwi->setThrottlePercentage(100.0);
			}
			else if (dist > 1) {
				hwi->setThrottlePercentage(dist * 4);
			}
			else {
				hwi->setThrottlePercentage(0);
			}

		}

		//hrt.reset();
		start = end;

		//Cap at 1000Hz. no need for this to run any faster and chew cycles
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
	return true;
}

/*
void* ControlUpdaterRunnable::run() {

	bool success = true;

	while (controller->isAlive()) {
		// no point in doing this as fast as humanly possible - the other end wont keep up
		// CAP AT 1000Hz
		//SDL_Delay(1);

		success = controller->updateLoop();

		if (!success) {
			return (void*)false;
		}
	}

	return (void*)true;
}
*/