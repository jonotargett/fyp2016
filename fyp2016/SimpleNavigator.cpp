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
	
	// TODO(): subdivide() here for testing purposes at the moment
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


/*
subdivide: takes user defined waypoints and subdivides them with waypoints
spaced apart as defined in variables at start of function.  1 unit represents
1m. function loops through each line segment. for each iteration the line
segment is subdivided and waypoints for a turn into the next line segment is
calculated and added.
*/
bool SimpleNavigator::subdivide() {
	// TODO(): adjust the units in this function when more info on GPS is available.

	double const distanceBetweenWaypoints = 0.5;
	double const distanceBetweenTurnWaypoints = 0.2;
	double const turnRadius = 3.14;
	std::vector<double> turnAngleListDegrees = {
		0, 26, 62.6, 81.5, 101.9, 125.1, 148.3, 180 };

	std::vector<Point> subdividedPath;

	//filling path with dummy points for testing purposes:
	Point dummyPoints = Point(30, 80);
	addPoint(dummyPoints);
	dummyPoints = Point(60, 100);
	addPoint(dummyPoints);
	dummyPoints = Point(60, 60);
	addPoint(dummyPoints);
	dummyPoints = Point(30, 50);
	addPoint(dummyPoints);

	// for each line segment (each line between two 'ultimate' waypoints)
	for (unsigned int i = 0; i < path.size() - 1; i++) {
		
		/*
			subdividing straight line segments:
			get a unit vector in the direction from the start point to the finish point.
			place a waypoint at each specified distance using the unit vector.
		*/

		Point curPoint = *path.at(i);
		Point nexPoint = *path.at(i + 1);
		Point directionVector = Point(nexPoint.x - curPoint.x, 
									nexPoint.y - curPoint.y);
		directionVector.normalise();

		// intermediate holds the current intermediate waypoint
		Point intermediate = Point(curPoint.x, curPoint.y);

		// FactorXY used for determining whether intermediate point is still within boundary points.
		double FactorX = (directionVector.x != 0) ? directionVector.x / abs(directionVector.x) : 0;
		double FactorY = (directionVector.y != 0) ? directionVector.y / abs(directionVector.y) : 0;
		
		// while our intermediate point is still between the two 'ultimate' waypoints
		// intermediate is calculated based on curPoint to remove accumulative error
		int index = 0;
		while (intermediate.x * FactorX <= nexPoint.x * FactorX && intermediate.y * FactorY <= nexPoint.y * FactorY) {
			Point p = Point(intermediate.x, intermediate.y);
			subdividedPath.push_back(p);
			index++;
			intermediate.x = curPoint.x + index * (directionVector.x * distanceBetweenWaypoints);
			intermediate.y = curPoint.y + index * (directionVector.y * distanceBetweenWaypoints);
		}
		subdividedPath.push_back(Point(nexPoint.x, nexPoint.y));


		/*
			waypoints for turn subdivision calculated below
			turn orients the quadbike such that its heading matches the heading
			of the next line segment. The main for loop (iterating on i) can
			then proceed.
			uses a vector from the centre of the turn circle to the quadbike to
			calculate waypoints. the vector is incrementally rotated and a
			waypoint added at each step.
		*/
		
		// if there is a next line segment to line up with:
		if (i + 2 < path.size()) {
			
			// figure out the turn angle
			double angle1 = atan2(curPoint.y - nexPoint.y, curPoint.x - nexPoint.x);
			double angle2 = atan2(nexPoint.y - path.at(i + 2)->y, nexPoint.x - path.at(i + 2)->x);
			
			// positive is clockwise. turn angle from -180 to 180
			double turnAngle = (angle1 - angle2);
			if (turnAngle<0) {
				turnAngle += 2 * PI;
			}
			if (turnAngle > PI) {
				turnAngle -= 2 * PI;
			}
			cout << turnAngle * 180/PI << endl;
			
			// current path angle, clockwise from positive y (note, we are using 
			// cartesian coordinates +y is up, +x is right).
			double currentAngle = (atan2(nexPoint.y - curPoint.y, nexPoint.x - curPoint.x) - PI / 2) * -1;
			
			// convert turnAngleListDegrees to radians
			std::vector<double> turnAngleListRads;
			for (unsigned int m = 0; m < turnAngleListDegrees.size(); m++) {
				turnAngleListRads.push_back(turnAngleListDegrees.at(m) * PI/180);
			}
			
			// dirVector holds the vector from the centre of the turning circle
			// to the quad bike. needs some manipulation before we have the
			// correct vector.
			// get direction vector from initial point to final point of line segment

			Point dirVector = Point(nexPoint.x - curPoint.x, nexPoint.y - curPoint.y);
			dirVector.normalise();
			dirVector.x *= turnRadius;	// turn radius
			dirVector.y *= turnRadius;	// turn radius

			// first turn arc, a LEFT turn has negative y
			double tempX = dirVector.x;
			dirVector.x = dirVector.y;
			dirVector.y = -tempX;

			double centreX = nexPoint.x - dirVector.x;
			double centreY = nexPoint.y - dirVector.y;

			bool hasReachedCorrectAngle = false;

			// j cycles through each of the turns in the N-point turn
			for (int j = 0; j < 7; j++) {

				// k increments the angle and places multiple waypoints for each of the turns.
				double increment = distanceBetweenTurnWaypoints / turnRadius;
				for (double k = increment; k < (turnAngleListRads.at(j + 1) - turnAngleListRads.at(j)); k += increment) {
					double newVecX = cos(k) * dirVector.x + sin(k) * dirVector.y;
					double newVecY = -sin(k) * dirVector.x + cos(k) * dirVector.y;
					
					if (k + turnAngleListRads.at(j) > turnAngle) {
						hasReachedCorrectAngle = true;
						break;
					}

					Point pp = Point(centreX + newVecX, centreY + newVecY);
					subdividedPath.push_back(pp);
					
				}
				if (hasReachedCorrectAngle) {
					break;
				}
				//adding the last point that the for loop would have missed.
				double finalTurnAngle = (turnAngleListRads.at(j + 1) - turnAngleListRads.at(j));
				double newVecX = cos(finalTurnAngle) * dirVector.x + sin(finalTurnAngle) * dirVector.y;
				double newVecY = -sin(finalTurnAngle) * dirVector.x + cos(finalTurnAngle) * dirVector.y;

				Point pp = Point(centreX + newVecX, centreY + newVecY);
				subdividedPath.push_back(pp);

				if (finalTurnAngle + turnAngleListRads.at(j) > turnAngle) {
					hasReachedCorrectAngle = true;
					break;
				}

				// set up direction vector for the next arc in the n-point turn
				double rotationAngle = turnAngleListRads.at(j + 1) - turnAngleListRads.at(j);
				tempX = cos(rotationAngle) * dirVector.x + sin(rotationAngle) * dirVector.y;
				dirVector.y = -sin(rotationAngle) * dirVector.x + cos(rotationAngle) * dirVector.y;
				dirVector.x = tempX;

				//then flip it so we turn the other way
				dirVector.y *= -1;
				dirVector.x *= -1;

				centreX -= 2 * dirVector.x;
				centreY -= 2 * dirVector.y;
			}

			// this is the distance that needs to be corrected for in the y direction to make the quad colinear with the next line segment
			double deltaY = 0.5843*pow(turnAngle, 4) - 3.1669*pow(turnAngle, 3) + 5.968*pow(turnAngle, 2) - 4.047*turnAngle + 0.1295;
		}



	}

	/*
	TODO (JONO could you please check that ive done this right or correct it if i havnt:)
	*/
	for (unsigned int i = 0; i < path.size(); i++) {
		delete path.at(i);
	}
	path.clear();
	for (unsigned int i = 0; i < subdividedPath.size(); i++) {
		addPoint(subdividedPath.at(i));
	}
	subdividedPath.clear();
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