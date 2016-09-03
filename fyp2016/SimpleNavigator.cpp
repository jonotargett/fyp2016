#include "SimpleNavigator.h"



SimpleNavigator::SimpleNavigator()
{
	simpleTurnMaxAngleRad = 50 * PI / 180;
	//simpleTurnMaxAngleRad = 0;
	baseLoc = LatLng(-34.919, 138.603);
}


SimpleNavigator::~SimpleNavigator()
{
}


bool SimpleNavigator::initialise() {
	
	//filling path with dummy points for testing purposes:
	Point dummyPoints = Point(-2.5, -3);
	addPoint(dummyPoints);
	dummyPoints = Point(-4, 1.5);
	addPoint(dummyPoints);
	dummyPoints = Point(0, 1.5);
	addPoint(dummyPoints);
	dummyPoints = Point(0, -3);
	addPoint(dummyPoints);
	dummyPoints = Point(0, -2);
	addPoint(dummyPoints);
	dummyPoints = Point(5, 4);
	addPoint(dummyPoints);
	

	Log::d << "Navigator initialised." << std::endl;
	return true;
}

void SimpleNavigator::clearPath() {

	path.clear();
}

void SimpleNavigator::setPath(std::vector<Point>) {

}


void SimpleNavigator::setBaseLocation(LatLng ll) {
	baseLoc = ll;
}


/*
addPoint: entire path should be cleared before adding more points (call clearPath()),
else when we go to subdivide it is going to try and subdivide points which have 
already been subdivided.
*/
void SimpleNavigator::addPoint(Point p) {
	path.push_back(p);
}
void SimpleNavigator::addPoint(LatLng ll) {
	Point rel = ll.relativeFrom(baseLoc);

	path.push_back(rel);
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

	double const distanceBetweenWaypoints = 0.1;
	double const distanceBetweenTurnWaypoints = 0.1;
	double const turnRadius = 3.14;		// max turn radius of the quad bike
	
	// heading of quad bike at each point of an N-point turn
	std::vector<double> turnAngleListDegrees = {
		0, 26, 62.6, 81.5, 101.9, 125.1, 148.3, 180 };

	//std::vector<Point> subdividedPath;
	subdividedPath.clear();
	

	// for each line segment (each line between two 'ultimate' waypoints)
	Point curPoint = path.at(0);
	Point nexPoint = path.at(1);
	for (unsigned int i = 0; i < path.size() - 1; i++) {
		
		/*
			subdividing straight line segments:
			get a unit vector in the direction from the start point to the finish point.
			place a waypoint at each specified distance using the unit vector.
		*/

		//Point curPoint = *path.at(i);
		//Point nexPoint = *path.at(i + 1);
		
		nexPoint = path.at(i + 1);

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

		// add the last ultimate waypoint if its not already there
		if (subdividedPath.at(subdividedPath.size() - 1).x != nexPoint.x || subdividedPath.at(subdividedPath.size() - 1).y != nexPoint.y)
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
			double angle2 = atan2(nexPoint.y - path.at(i + 2).y, nexPoint.x - path.at(i + 2).x);
			
			// positive is clockwise. turn angle from -pi to pi
			double turnAngle = (angle1 - angle2);
			if (turnAngle<0) {
				turnAngle += 2 * PI;
			}
			if (turnAngle > PI) {
				turnAngle -= 2 * PI;
			}

			if (abs(turnAngle) <= simpleTurnMaxAngleRad) {// conduct a simple turn

				// distance from waypoint to begin conducting hte turn
				double d = abs(turnRadius * tan(turnAngle / 2));
				Point endPoint = subdividedPath.at(subdividedPath.size() - 1);
				double distanceFromRemoved = 0;

				// remove the last few points from the straight line segment up to a distance d from the turn waypoint
				while (distanceFromRemoved <= d) {
					subdividedPath.erase(subdividedPath.end() - 1);
					distanceFromRemoved = endPoint.getDistanceTo(subdividedPath.at(subdividedPath.size() - 1));
				}
				
				// add a point a distance d from the endpoint
				Point dPoint = Point(endPoint.x + -d * directionVector.x, endPoint.y + -d * directionVector.y);
				subdividedPath.push_back(dPoint);


				// dirVector holds the vector from the centre of the turning circle
				// to the quad bike. needs some manipulation before we have the
				// correct vector.
				// get direction vector from initial point to final point of line segment
				Point dirVector = Point(nexPoint.x - curPoint.x, nexPoint.y - curPoint.y);
				dirVector.normalise();
				dirVector.x *= turnRadius;
				dirVector.y *= turnRadius;

				// factor to multiply angles and such. if a right turn = -1, if left = 1
				int turnFactor = (turnAngle >= 0) ? -1 : 1;

				// first turn arc, a RIGHT turn arc has negative x, a LEFT turn arc has negative y
				double tempX = dirVector.x;
				dirVector.x = dirVector.y * turnFactor;
				dirVector.y = -tempX * turnFactor;

				// centre of the turning circle
				double centreX = dPoint.x - dirVector.x;
				double centreY = dPoint.y - dirVector.y;



				// k increments the angle and places multiple waypoints for each of the turns.
				// increment holds the change in angle needed
				double increment = distanceBetweenTurnWaypoints / turnRadius;
				for (double k = increment; k < abs(turnAngle); k += increment) {

					double newVecX = cos(k * turnFactor) * dirVector.x - sin(k * turnFactor) * dirVector.y;
					double newVecY = sin(k * turnFactor) * dirVector.x + cos(k * turnFactor) * dirVector.y;

					if (k > abs(turnAngle)) {
						break;
					}

					Point pp = Point(centreX + newVecX, centreY + newVecY);
					subdividedPath.push_back(pp);
				}

			}
			else {// conduct N-Point turn
				
				// add a point to adjust for the distance set off from the next line segment
				
				//used to get deltaY from an approximate function, now uses linear interpolation from getDeltaY function
				//double deltaY = 0.5843*pow(abs(turnAngle), 4) - 3.1669*pow(abs(turnAngle), 3) + 5.968*pow(abs(turnAngle), 2) - 4.047*abs(turnAngle) + 0.1295;
				
				double deltaY = getDeltaY(abs(turnAngle));
				if (deltaY > 0.2) deltaY = 0.2;
				Point turnPoint;
				turnPoint.x = nexPoint.x - directionVector.x*deltaY;
				turnPoint.y = nexPoint.y - directionVector.y*deltaY;
				subdividedPath.push_back(Point(turnPoint.x, turnPoint.y));

				// current path angle, clockwise from positive y (note, we are using 
				// cartesian coordinates +y is up, +x is right).
				double currentAngle = (atan2(nexPoint.y - curPoint.y, nexPoint.x - curPoint.x) - PI / 2) * -1;

				// convert turnAngleListDegrees to radians
				std::vector<double> turnAngleListRads;
				for (unsigned int m = 0; m < turnAngleListDegrees.size(); m++) {
					turnAngleListRads.push_back(turnAngleListDegrees.at(m) * PI / 180);
				}

				// dirVector holds the vector from the centre of the turning circle
				// to the quad bike. needs some manipulation before we have the
				// correct vector.
				// get direction vector from initial point to final point of line segment

				Point dirVector = Point(nexPoint.x - curPoint.x, nexPoint.y - curPoint.y);
				dirVector.normalise();
				dirVector.x *= turnRadius;
				dirVector.y *= turnRadius;

				// factor to multiply angles and such. if a right turn = 1, if left = -1
				int turnFactor = (turnAngle >= 0) ? 1 : -1;

				// first turn arc, a RIGHT turn arc has negative x, a LEFT turn arc has negative y
				double tempX = dirVector.x;
				dirVector.x = dirVector.y * turnFactor;
				dirVector.y = -tempX * turnFactor;

				// centre of the turning circle
				double centreX = turnPoint.x - dirVector.x;
				double centreY = turnPoint.y - dirVector.y;

				bool hasReachedCorrectAngle = false;

				// j cycles through each of the turns in the N-point turn
				for (int j = 0; j < 7; j++) {

					// k increments the angle and places multiple waypoints for each of the turns.
					// increment holds the change in angle needed
					double increment = distanceBetweenTurnWaypoints / turnRadius;
					for (double k = increment; k < (turnAngleListRads.at(j + 1) - turnAngleListRads.at(j)); k += increment) {
						double newVecX = cos(k * turnFactor) * dirVector.x + sin(k * turnFactor) * dirVector.y;
						double newVecY = -sin(k * turnFactor) * dirVector.x + cos(k * turnFactor) * dirVector.y;

						if (k + turnAngleListRads.at(j) > turnAngle * turnFactor) {
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
					double newVecX = cos(finalTurnAngle * turnFactor) * dirVector.x + sin(finalTurnAngle * turnFactor) * dirVector.y;
					double newVecY = -sin(finalTurnAngle * turnFactor) * dirVector.x + cos(finalTurnAngle * turnFactor) * dirVector.y;

					Point pp = Point(centreX + newVecX, centreY + newVecY);
					subdividedPath.push_back(pp);

					if (finalTurnAngle + turnAngleListRads.at(j) > turnAngle * turnFactor) {
						hasReachedCorrectAngle = true;
						break;
					}

					// set up direction vector for the next arc in the n-point turn
					double rotationAngle = turnAngleListRads.at(j + 1) - turnAngleListRads.at(j);
					double tempX = cos(rotationAngle * turnFactor) * dirVector.x + sin(rotationAngle * turnFactor) * dirVector.y;
					dirVector.y = -sin(rotationAngle * turnFactor) * dirVector.x + cos(rotationAngle * turnFactor) * dirVector.y;
					dirVector.x = tempX;

					//then flip it so we turn the other way
					dirVector.y *= -1;
					dirVector.x *= -1;

					centreX -= 2 * dirVector.x;
					centreY -= 2 * dirVector.y;
				}
			}

		}
		curPoint = subdividedPath.at(subdividedPath.size() - 1);
	}

	/*
	for (unsigned int i = 0; i < path.size(); i++) {
		delete path.at(i);
	}
	path.clear();
	for (unsigned int i = 0; i < subdividedPath.size(); i++) {
		addPoint(subdividedPath.at(i));
		//Log::i << subdividedPath.at(i).x << ", " << subdividedPath.at(i).y << endl;
	}
	subdividedPath.clear();
	*/
	Log::d << "Path subdivision completed" << endl;
	return false;
}

/*
returns the distance that the quad bike is out after completing a turn
to line up with the next line segment based off a list of experimental
values (interpolation)
*/
double SimpleNavigator::getDeltaY(double radians) {
	/* deltaYList: magic numbers
	these come from experimental data (google sketchup drawings).
	x values store the heading at which each point in the N-point turn
	is complete without the quadbike leaving the swathe
	y values store the y-error at each of the end points in the N-point turn
	the y-error is the error distance the quad bike would be shifted from
	the next portion of hte path if the quad bike were to drive at the
	exact heading given by x.

	* represents quad x, y.
	quad heading would be equal to the heading of the second path at this point


	/
	/
	-	* /
	y-error |	 /
	-	/
	|
	|
	|
	|
	|
	*/

	std::vector<Point> deltaYList = { Point(0, 0),
		Point(26,-0.724),
		Point(62.6,-0.51),
		Point(81.5,-0.3),
		Point(101.9,-0.07),
		Point(125.1,0.2),
		Point(148.3,0.7),
		Point(165.8,2.72),
		Point(180, 3)};
	double degrees = abs(radians * 180 / PI);
	int i = 0;
	while (degrees > deltaYList.at(i).x) {
		i++;
		if (i > (int)deltaYList.size() - 1) {
			i--;
			break;
		}
	}
	double deltaY = deltaYList.at(i).y +
		((degrees - deltaYList.at(i).x) / (deltaYList.at(i - 1).x - deltaYList.at(i).x))
		*(deltaYList.at(i - 1).y - deltaYList.at(i).y);
	return deltaY;
}

bool SimpleNavigator::startPath() {

	updater = new std::thread(&SimpleNavigator::loop, this);
	return true;
}

std::vector<Point> SimpleNavigator::getPath() {
	return subdividedPath;
}


void SimpleNavigator::loop() {
	endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> seconds = endTime - startTime;

	if (path.size() <= 0)
		return;

	while (true) {
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}

	
}

