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

	////// 1 unit here is equivalent to 1m, will need to be adjusted when more info on GPS is available.

	double distanceBetweenWaypoints = 2;

	//filling path with dummy points for testing purposes:
	Point dummyPoints = Point(50, 10);
	addPoint(dummyPoints);
	dummyPoints = Point(30, 80);
	addPoint(dummyPoints);
	dummyPoints = Point(60, 50);
	addPoint(dummyPoints);
	dummyPoints = Point(130, 100);
	addPoint(dummyPoints);

	std::vector<Point> subdividedPath;

	// for each line segment (each line between two 'ultimate' waypoints)
	for (int i = 0; i < path.size() - 1; i++) {

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
			Point p = Point(intermediate.x, intermediate.y);
			subdividedPath.push_back(p);
			// add to initial point rather than incrementing 'intermediate' to remove accumulative error
			k++;
			intermediate.x = path.at(i)->x + k * (directionVector.x * distanceBetweenWaypoints);
			intermediate.y = path.at(i)->y + k * (directionVector.y * distanceBetweenWaypoints);
		}

		///
		/// waypoints for turn subdivision below: see document on google drive for more information on how this is calculated
		/// the turn will orient the quadbike such that its heading matches the new heading of the next line segment.  The main for loop can then iterate to work on the next line segment.
		///
		// turn code goes here!!!!!
		//angle between 2 lines:

		// if we still have a turn to calculate:
		if (i + 2 < path.size()) {
			
			// figure out the turn angle
			double angle1 = atan2(path.at(i)->y - path.at(i + 1)->y, path.at(i)->x - path.at(i + 1)->x);
			double angle2 = atan2(path.at(i + 1)->y - path.at(i + 2)->y, path.at(i + 1)->x - path.at(i + 2)->x);
			
			// positive is clockwise. turn angle from -180 to 180
			double turnAngle = (angle1 - angle2);
			if (turnAngle<0) {
				turnAngle += 2 * 3.14159265;
			}
			if (turnAngle > 3.14159265) {
				turnAngle -= 2 * 3.14159265;
			}
			
			//currentAngle, clockwise from positive y (note, we are using cartesian coordinates +y is up, +x is right).
			double currentAngle = (atan2(path.at(i+1)->y - path.at(i)->y, path.at(i+1)->x - path.at(i)->x) - 3.14159265/2) * -1;

			Point dirVector = Point(path.at(i + 1)->x - path.at(i)->x, path.at(i + 1)->y - path.at(i)->y);
			dirVector.normalise();
			dirVector.x *= 3.14;	// turn radius
			dirVector.y *= 3.14;	// turn radius

			// first turn arc, left turn has negative y
			double tempX = dirVector.x;
			dirVector.x = dirVector.y;
			dirVector.y = -tempX;

			

			double centreX = path.at(i+1)->x - dirVector.x;
			double centreY = path.at(i+1)->y - dirVector.y;

			double newVecX = cos(26) * dirVector.x + sin(26) * dirVector.y;
			double newVecY = -sin(26) * dirVector.x + cos(26) * dirVector.y;

			Point pp = Point(centreX + newVecX, centreY + newVecY);

			// this is the first point of the N-point turn
			double oldX = -0.32;
			double oldY = -1.38;
			double newwX = cos(currentAngle) * oldX + sin(currentAngle) * oldY;
			double newwY = -sin(currentAngle) * oldX + cos(currentAngle) * oldY;

			Point p = Point(path.at(i+1)->x + newwX, path.at(i+1)->y + newwY);
			subdividedPath.push_back(pp);
			


			// this is the distance that needs to be corrected for in the y direction to make the quad colinear with the next line segment
			double deltaY = 0.5843*pow(turnAngle, 4) - 3.1669*pow(turnAngle, 3) + 5.968*pow(turnAngle, 2) - 4.047*turnAngle + 0.1295;
		}



	}

	/*
	JONO could you please check that ive done this right or correct it if i havnt:
	*/
	for (int i = 0; i < path.size(); i++) {
		delete path.at(i);
	}
	for (int i = 0; i < subdividedPath.size(); i++) {
		addPoint(subdividedPath.at(i));
	}

	Log::i << "Path subdivision completed" << endl;
	return false;
}

bool SimpleNavigator::startPath() {

	updater = new std::thread(&SimpleNavigator::loop, this);
	return true;
}

std::vector<Point*> SimpleNavigator::getPath() {
	return path;
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