#pragma once
#include "NavigationSystem.h"

#include <thread>
#include <chrono>

#define PI 3.1415926535

class SimpleNavigator :
	public NavigationSystem
{
private:
	DriveController* dc;
	HardwareInterface* hwi;
	std::vector<Point*> path;


	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

	std::thread* updater;

public:
	SimpleNavigator();
	~SimpleNavigator();

	bool initialise(DriveController*, HardwareInterface*);
	void clear();

	void setPath(std::vector<Point*>);
	std::vector<Point*> getPath();
	void addPoint(Point);

	bool subdivide();
	bool startPath();

	void loop();

};

