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

public:
	SimpleNavigator();
	~SimpleNavigator();

	bool initialise();
	void clearPath();
	void clearSubdividedPath();

	void setBaseLocation(Point);
	void setPath(std::vector<Point>);
	std::vector<Point>* getSubdividedPath();
	void addPoint(Point);
	void addPoint(LatLng);

	// returns true if there is a next point in the path
	bool updatePoint(Point position, float heading, float velocity);
	// get the current point to steer to
	Point getPoint();
	// is the quad travelling in the forwards direction (mainly for defining turns)
	// done by checking if the point is in front or behind of quad bike now.
	//bool getIsForwards();
	// does the point exist when the argument is added to the currentPathPointIndex
	bool isNextPoint();
	bool isConverging();

	void renewPath();
	NavState getState();
	void addLatLongPoints(std::vector<Point>);

	bool subdivide(Point quadPosition, float heading);
	bool startPath();

	void loop();

private:
	std::vector<Point> path;
	std::vector<Point> subdividedPath;
	LatLng baseLoc;

	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

	std::thread* updater;
	double getDeltaY(double radians);

	// previous linear point is basically the last 'path' point before hte turn point (turnIndexPont)
	// used to determine if a simpleturn is possible based on the distance back to it.
	bool simpleTurn(Point heading, double turnAngle, int turnIndexPoint, Point previousLinearPoint);
	bool nPointTurn(Point heading, double turnAngle, Point turnIndexPoint);

	Point currentPathPoint;
	unsigned int currentPathPointIndex;
	unsigned int turnPoint;
	unsigned int nextTurnPoint;
	bool travelPathForwards;
	//bool isForwards;
	NavState navState;
	Point baseLocation;
	bool pathNavigationCompleted;
	double distanceToTurn;
	bool converging;
	double initialDistanceToTurn;

	double simpleTurnMaxAngleRad;
	double noTurnMaxRads;
	double distanceBetweenWaypoints;			// meters
	double distanceBetweenTurnWaypoints;
	double minTurnRadius;		// min turn radius of the quad bike



};

