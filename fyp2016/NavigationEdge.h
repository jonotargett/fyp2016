#pragma once
#include "Point.h"

class NavigationEdge
{
private:
	Point origin;
	Point destination;
	bool shouldTravelForward;

public:
	NavigationEdge();
	NavigationEdge(Point o, Point d);
	NavigationEdge(Point o, Point d, bool direction);
	~NavigationEdge();

	Point getOrigin();
	Point getDestination();
	bool isForwards();
};

