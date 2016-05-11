#pragma once

#include <vector>
#include "Point.h"
#include "DriveController.h"

class NavigationSystem
{
public:

	/* Initialises the navigation system.
	Returns: true on success, false on failure
	*/
	virtual bool initialise(DriveController*) = 0;

	/* Clears all current paths, zones etc.
	Returns:
	*/
	virtual void clear() = 0;

	/*
	sets the navigation path to be this list of points, in the order given
	*/
	virtual void setPath(std::vector<Point*>) = 0;

	/*
	adds the current point to the navigation path, appending to the end of the list
	*/
	virtual void addPoint(Point) = 0;


	/*
	Prepares the list of points into a path that can be followed by the actual vehicle.
	This is a preprocess (happens before vehicle starts moving) so that errors can be found
	before motion begins.
	HARRISON: your stuff goes into this method
	*/
	virtual bool subdivide() = 0;


	/*
	Starts the vehicle and continues running until the vehicle has completed the path.
	Returns: true if successful completion, false if halted for any reason
	*/
	virtual bool startPath() = 0;
};

