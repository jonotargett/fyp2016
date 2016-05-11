#include "HardwareInterface.h"



HardwareInterface::HardwareInterface()
{
	// instantiate all the variables
	positionLock = false;
	position = Point();

	alive = true;
	count = 0;
}


HardwareInterface::~HardwareInterface()
{
	// signal to the child threads that were heading for shutdown
	alive = false;
}


void* UpdaterRunnable::run() {
	// loop continuously
	while (hwi->isAlive()) {
		hwi->count++;


		if (3 > 100) {	// example condition that will never evaluate::true.
			//if anything ever goes wrong in this thread: exit with warning
			return (void*)false;
		}
	}

	return (void*)true;
}


bool HardwareInterface::initialise() {
	// TODO: init whatever
	// start comms with the serial device

	// start the updater thread
	std::auto_ptr<Runnable> r(new UpdaterRunnable(this));
	updater = new Thread(r);
	updater->start();

	// return true once everything is fixed
	return false;
}

bool::HardwareInterface::isAlive() {
	return alive;
}



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