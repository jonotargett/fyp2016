#pragma once
#include "NavigationSystem.h"
class SimpleNavigator :
	public NavigationSystem
{
private:
	DriveController* dc;
public:
	SimpleNavigator();
	~SimpleNavigator();

	bool initialise(DriveController*);
	void clear();

	void setPath(std::vector<Point*>);
	void addPoint(Point);

	bool subdivide();
	bool startPath();


};

