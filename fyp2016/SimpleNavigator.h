#pragma once
#include "NavigationSystem.h"

#include "Log.h"

#include <thread>
#include <chrono>

#ifndef PI
#define PI 3.1415926535
#endif


enum NavState {
	NAV_CRUISE,
	NAV_TURNINBOUND,
	NAV_LANDMINE_DETECTED,
	NAV_WAIT
};

class SimpleNavigator :
	public NavigationSystem
{
private:
	std::vector<Point*> path;

	double simpleTurnMaxAngleRad;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

	std::thread* updater;
	double getDeltaY(double radians);

	unsigned int currentPathPoint;
	unsigned int turnPoint;
	unsigned int nextTurnPoint;
	bool travelPathForwards;
	bool isForwards;
	NavState navState;


public:
	SimpleNavigator();
	~SimpleNavigator();

	bool initialise();
	void clearPath();

	void setPath(std::vector<Point*>);
	std::vector<Point*> getPath();
	void addPoint(Point);

	// returns true if there is a next point in the path
	bool updatePoint(Point position, float heading);
	// get the current point to steer to
	Point getPoint();
	// is the quad travelling in the forwards direction (mainly for defining turns)
	bool getIsForwards();
	// does the point exist when the argument is added to the currentPathPoint
	bool isNextPoint();

	bool subdivide(Point quadPosition, float heading);
	bool startPath();

	void loop();

};

