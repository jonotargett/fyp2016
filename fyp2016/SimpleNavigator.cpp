#include "SimpleNavigator.h"



SimpleNavigator::SimpleNavigator()
{
}


SimpleNavigator::~SimpleNavigator()
{
}


bool SimpleNavigator::initialise(DriveController* controller, HardwareInterface* hardware) {
	dc = controller;
	hwi = hardware;
	
	// subdivide() here for testing purposes at the moment
	subdivide();

	Log::i << "Navigator initialised." << std::endl;
	return true;
}

void SimpleNavigator::clear() {

}

void SimpleNavigator::setPath(std::vector<Point*>) {

}

void SimpleNavigator::addPoint(Point p) {
	Point* np = new Point(p.x, p.y);

	path.push_back(np);
}

bool SimpleNavigator::subdivide() {

	////// 1 unit here is equivalent to 10cm, will need to be adjusted when more info on GPS is available.

	double distanceBetweenWaypoints = 2;

	//filling path with dummy points for testing purposes:
	Point dummyPoints = Point(30, 10);
	addPoint(dummyPoints);
	dummyPoints = Point(20, 50);
	addPoint(dummyPoints);
	dummyPoints = Point(60, 80);
	addPoint(dummyPoints);
	dummyPoints = Point(80, 20);
	addPoint(dummyPoints);

	std::vector<Point*> subdividedPath;

	// for each line segment (each line between two 'ultimate' waypoints)
	for (int i = 0; i < (int)path.size() - 1; i++) {
		///
		///	subdividing straight line segments:
		/// get a unit vector in the direction from the start point to the finish point. place a waypoint at each specified distance using the unit vector.
		///
		Point directionVector = Point(path.at(i + 1)->x - path.at(i)->x, path.at(i + 1)->y - path.at(i)->y);
		directionVector.normalise();

		// intermediate holds the current intermediate waypoint
		Point intermediate = Point(path.at(i)->x, path.at(i)->y);

		// FactorXY used for determining whether intermediate point is still within boundary points.
		double FactorX = directionVector.x / abs(directionVector.x);
		double FactorY = directionVector.y / abs(directionVector.y);
		int k = 0;
		//while our intermediate point is still between the two 'ultimate' waypoints
		while (intermediate.x * FactorX <= path.at(i + 1)->x * FactorX && intermediate.y * FactorY <= path.at(i + 1)->y * FactorY) {
			Point* p = new Point(intermediate.x, intermediate.y);
			subdividedPath.push_back(p);
			// add to initial point rather than incrementing 'intermediate' to remove accumulative error
			k ++;
			intermediate.x = path.at(i)->x + k * (directionVector.x * distanceBetweenWaypoints);
			intermediate.y = path.at(i)->y + k * (directionVector.y * distanceBetweenWaypoints);
		}

		///
		/// waypoints for turn subdivision below: see document on google drive for more information on how this is calculated
		/// the turn will orient the quadbike such that its heading matches the new heading of the next line segment.  The main for loop can then iterate to work on the next line segment.
		///

		// turn code goes here!!!!!

	}

	/*int k = 0;
	while (k < subdividedPath.size()) {
		cout << subdividedPath.at(k)->x << endl;
		k++;
	}
	k = 0;
	cout << endl;
	while (k < subdividedPath.size()) {
		cout << subdividedPath.at(k)->y << endl;
		k++;
	}*/

	Log::i << "Path subdivision completed" << endl;
	return false;
}

bool SimpleNavigator::startPath() {

	updater = new std::thread(&SimpleNavigator::loop, this);
	return true;
}


void SimpleNavigator::loop() {
	endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> seconds = endTime - startTime;

	Point curPos = hwi->getPosition();

	if (path.size() <= 0)
		return;

	Point* nextPos = path.at(0);
	Point dif = Point(nextPos->x, nextPos->y);
	dif = Point(dif.x - curPos.x, dif.y - curPos.y);
	double dist = sqrt(dif.x * dif.x + dif.y*dif.y);

	while (true) {
		curPos = hwi->getPosition();
		dif = Point(nextPos->x, nextPos->y);
		dif = Point(dif.x - curPos.x, dif.y - curPos.y);
		dist = sqrt(dif.x * dif.x + dif.y*dif.y);


		dc->setInputs(0, dist);

		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}

	
}