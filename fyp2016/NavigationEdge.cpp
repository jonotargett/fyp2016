#include "NavigationEdge.h"



NavigationEdge::NavigationEdge()
{
	shouldTravelForward = true;
	origin = Point();
	destination = Point();
}

NavigationEdge::NavigationEdge(Point o, Point d) : origin(o), destination(d) {
	shouldTravelForward = true;
}

NavigationEdge::NavigationEdge(Point o, Point d, bool direction) : origin(o), destination(d), shouldTravelForward(direction) {
}


NavigationEdge::~NavigationEdge()
{
}


Point NavigationEdge::getOrigin() {
	return origin;
}

Point NavigationEdge::getDestination() {
	return destination;
}

bool NavigationEdge::isForwards() {
	return shouldTravelForward;
}