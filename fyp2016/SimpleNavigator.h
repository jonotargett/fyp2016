#pragma once
#include "NavigationSystem.h"

#include "Log.h"

#include <thread>
#include <chrono>

#ifndef PI
#define PI 3.1415926535
#endif

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

public:
	SimpleNavigator();
	~SimpleNavigator();

	bool initialise();
	void clearPath();

	void setPath(std::vector<Point*>);
	std::vector<Point*> getPath();
	void addPoint(Point);

	bool subdivide();
	bool startPath();

	void loop();

};

