#pragma once

#include <vector>
#include "Point.h"
#include "LatLng.h"

enum NavState {
	NAV_CRUISE,
	NAV_TURNINBOUND,
	NAV_LANDMINE_DETECTED,
	NAV_WAIT
};

class NavigationSystem
{
public:
	
	/* Initialises the navigation system.
	Returns: true on success, false on failure
	*/
	virtual bool initialise() = 0;

	/* Clears all current paths, zones etc.
	Returns:
	*/
	virtual void clearPath() = 0;
	virtual void clearSubdividedPath() = 0;
	virtual void addLatLongPoints(std::vector<Point>) = 0;

	/*
	sets the position (in LatLng) of the relative calculations
	so that everything else can be transformed to metres
	*/
	virtual void setBaseLocation(Point) = 0;

	/*
	sets the navigation path to be this list of points, in the order given
	*/
	virtual void setPath(std::vector<Point>) = 0;

	/*
	gets the current point to turn to
	*/
	virtual Point getPoint() = 0;

	virtual NavState getState() = 0;

	virtual void renewPath() = 0;

	/*
	adds the current point to the navigation path, appending to the end of the list
	*/
	virtual void addPoint(Point) = 0;
	virtual void addPoint(LatLng) = 0;

	/*
	returns the navigation path
	*/
	virtual std::vector<Point>* getSubdividedPath() = 0;
	
	/*
	Prepares the list of points into a path that can be followed by the actual vehicle.
	This is a preprocess (happens before vehicle starts moving) so that errors can be found
	before motion begins.
	HARRISON: your stuff goes into this method
	*/
	virtual bool subdivide(Point, float) = 0;


	/*
	Starts the vehicle and continues running until the vehicle has completed the path.
	Returns: true if successful completion, false if halted for any reason
	*/
	virtual bool startPath() = 0;
};

