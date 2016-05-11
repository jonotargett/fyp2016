#pragma once
#include "HardwareInterface.h"

class DriveController
{
public:

	/*
	Initialise whatever
	Returns: true on success, false on failure
	*/
	virtual bool initialise(HardwareInterface*);

	/*
	Getters and setters of the controller enabler.
	If this device is 'enabled', it will attempt to drive the vehicle
	using the inputs sent to it (heading and distance). if it is not
	enabled, it will not react to inputs.
	*/
	virtual void setEnabled(bool);
	virtual bool isEnabled();


	/*
	Updates to match desired relative position from a navigator (parent class).

	the two inputs to the MIMO controller system. 
	relative heading: units of radians.
		heading will be zero when the vehicle is directly on course.
		if the vehicle must make a CCW turn to correct, heading will be positive;
		if the vehicle must make a CW turn to correct, heading will be negative.

					0 heading

		+ve		<	^	>	-ve heading
				 \	|  /
				  \	| /	
				   \|/
				|-------|
				|		|
				|		|
				|		|
				|-------|


	Distance: units of metres.
	distance will be positive if the vehicle is to be driving forwards.
	distance will be negative if the vehicle is to be driving in reverse (such as for 3-point turns).

	*/
	virtual bool setInputs(double relativeHeading, double distance);
};

