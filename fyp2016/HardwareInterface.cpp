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

	alive = true;
}


HardwareInterface::~HardwareInterface()
{
	// signal to the child threads that were heading for shutdown
	alive = false;
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
	while (vehicleAbsoluteHeadingLock) {
		//wait
	}
	return vehicleAbsoluteHeading;
}
void HardwareInterface::setAbsoluteHeading(double x) {
	while (vehicleAbsoluteHeadingLock) {
		//wait
	}
	vehicleAbsoluteHeadingLock = true;
	vehicleAbsoluteHeading = x;
	vehicleAbsoluteHeadingLock = false;
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


double HardwareInterface::getVelocityHeading() {
	while (velocityAbsoluteHeadingLock) {
		//wait
	}
	return velocityAbsoluteHeading;
}
void HardwareInterface::setVelocityHeading(double x) {
	while (velocityAbsoluteHeadingLock) {
		//wait
	}
	velocityAbsoluteHeadingLock = true;
	velocityAbsoluteHeading = x;
	velocityAbsoluteHeadingLock = false;
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

bool HardwareInterface::getBrake() {
	while (brakeLock) {
		//wait
	}
	return brake;
}
void HardwareInterface::setBrake(bool x) {
	while (brakeLock) {
		//wait
	}
	brakeLock = true;
	brake = x;
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

HardwareInterface::Gear HardwareInterface::getGear() {
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