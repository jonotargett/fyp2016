#pragma once

#include "Point.h"
#include "Runnable.h"
#include "Thread.h"

class HardwareInterface
{
private:
	/*
	variables holding sensor data
	*/
	bool alive;

	bool positionLock;
	Point position;
	Thread* updater;

public:
	long count;

	HardwareInterface();
	~HardwareInterface();

	bool isAlive();

	void increment();

	/*
	Obtain a link with the microcontroller over the serial link.
	Returns: true on success, false on failure
	*/
	bool initialise();

	/* 
	Returns: the current position as determined by the GPS unit
	*/
	Point getPosition();

	void setPosition(Point p);
};


class UpdaterRunnable : public Runnable {
private:
	HardwareInterface* hwi;
public:
	UpdaterRunnable(HardwareInterface* i) : hwi(i) {	}
	virtual void* run();
};