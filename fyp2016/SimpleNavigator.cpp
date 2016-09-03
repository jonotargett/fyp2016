#include "SimpleNavigator.h"



SimpleNavigator::SimpleNavigator()
{
	simpleTurnMaxAngleRad = 50 * PI / 180;
	isForwards = true;
	travelPathForwards = true;
	currentPathPoint = 0;
	navState = NAV_CRUISE;
	nextTurnPoint = 0;
}


SimpleNavigator::~SimpleNavigator()
{
}


bool SimpleNavigator::initialise() {
	// doing this because i have no idea how to poperly initialise the vector
	addPointBasic(Point(0, 0));

	Log::d << "Navigator initialised." << std::endl;
	return true;
	
}

void SimpleNavigator::clearPath() {
	for (unsigned int i = 0; i < path.size(); i++) {
		delete path.at(i);
	}
	path.clear();
}

void SimpleNavigator::setPath(std::vector<Point>) {

}

bool SimpleNavigator::updatePoint(Point position, float heading) {

	if (!isNextPoint()) 
		return false;

	float lookAheadDistance = 1.8;
	float turnTolerance = 0.2;
	int increment = 0;
	increment = (travelPathForwards) ? 1 : -1;
	
	// when incrementing the point, the distnace has to get bigger or smaller,
	// if the distance gets bigger, keep incremening until distance is bigger
	// than look ahead distance
	// if the distance gets smaller, we have a turn inbound. stop incrememting!
	
	float distanceNow = position.getDistanceTo(*path.at(currentPathPoint));
	float distanceNext = position.getDistanceTo(*path.at(currentPathPoint + increment));

	// while we are cruising, and less than the look ahead distnace
	while (navState == NAV_CRUISE && distanceNow < lookAheadDistance) {
		if (!isNextPoint()) {
			// have already incremented pathPoint by now
			return true;
		}
		currentPathPoint += increment;
		distanceNow = position.getDistanceTo(*path.at(currentPathPoint));
		distanceNext = position.getDistanceTo(*path.at(currentPathPoint + increment));

		if (distanceNow > distanceNext) {
			// we have a turn coming up
			navState = NAV_TURNINBOUND;
			turnPoint = currentPathPoint;
			break;
		}
	}
	// turn stuff
	// we know where the first turn point is at this stage, turnPoint
	// is there another turn point after this?:
	// if true, we have reached the turn point.
	if (distanceNow < turnTolerance && navState == NAV_TURNINBOUND) {
		isForwards = !isForwards;
		navState = NAV_CRUISE;
		
		// but do we have another turn???
		float nowTurnDistance = 0;
		float nextTurnDistance = 0;
		nextTurnPoint = turnPoint;
		
		do {
			nowTurnDistance = path.at(turnPoint)->getDistanceTo(*path.at(currentPathPoint));
			nextTurnDistance = path.at(turnPoint)->getDistanceTo(*path.at(currentPathPoint + increment));
			if (nowTurnDistance > nextTurnDistance) {
				
				// we have another turn coming up
				navState = NAV_TURNINBOUND;
				turnPoint = currentPathPoint;
				break;
			}
			if (!isNextPoint()) {
				// there is no next turn point because theres no next point
				break;
			}
			currentPathPoint += increment;
		} while (nowTurnDistance < lookAheadDistance);

		
		
	}
	return true;
}

Point SimpleNavigator::getPoint() {
	if (currentPathPoint < path.size()) {
		return *path.at(currentPathPoint);
	}
	else {
		return Point(NULL, NULL);
	}
	
}

bool SimpleNavigator::getIsForwards() {
	return isForwards;
}

bool SimpleNavigator::isNextPoint() {
	if (travelPathForwards) {
		if (currentPathPoint + 1 >= path.size() - 1) {
			return false;
		}
	}
	if (!travelPathForwards) {
		if (currentPathPoint - 1 < 0) {
			return false;
		}
	}
	return true;
}

/*
addPoint: entire path should be cleared before adding more points (call clearPath()),
else when we go to subdivide it is going to try and subdivide points which have 
already been subdivided.
*/
void SimpleNavigator::addPointBasic(Point p) {
	Point* np = new Point(p.x, p.y);
	path.push_back(np);
}

void SimpleNavigator::addPoint(LatLng) {

}


bool SimpleNavigator::startPath() {

	updater = new std::thread(&SimpleNavigator::loop, this);
	return true;
}

std::vector<Point*> SimpleNavigator::getPath() {
	return path;
}

void SimpleNavigator::setBaseLocation(LatLng) {

}

/*
subdivide: takes user defined waypoints and subdivides them with waypoints
spaced apart as defined in variables at start of function.  1 unit represents
1m. function loops through each line segment. for each iteration the line
segment is subdivided and waypoints for a turn into the next line segment is
calculated and added.
*/
bool SimpleNavigator::subdivide(Point quadPosition, float heading) {

	// need to connect the quadbike to the path, if it isnt already.
	// add a line which extends from the front of hte quad bike a small distance,
	// then connect that line to the first point in the path.
	double distanceToStart = quadPosition.getDistanceTo(*path.at(0));
	if (distanceToStart > 1) {
		path.insert(path.begin(), new Point(quadPosition.x + 0.1*sin(heading), quadPosition.y + 0.1*cos(heading)));
		path.insert(path.begin(), new Point(quadPosition.x, quadPosition.y));
	}
	else {
		Log::e << "Cannot start demining process as quadbike begins too close to an unscanned region";
		return false;
	}



	currentPathPoint = 0;
	double const distanceBetweenWaypoints = 0.1;
	double const distanceBetweenTurnWaypoints = 0.1;
	double const turnRadius = 3.25;		// max turn radius of the quad bike
	
	// heading of quad bike at each point of an N-point turn
	// currently based on a max turn angle of 21.5 degrees, 
	// -> turnRadius = 3.25m
	std::vector<double> turnAngleListDegrees = {
		0, 26.7, 60.5, 77.3, 95.6, 113.9, 130.8, 180 };

	std::vector<Point> subdividedPath;

	// for each line segment (each line between two 'ultimate' waypoints)
	Point curPoint = *path.at(0);
	Point nexPoint = *path.at(1);
	for (unsigned int i = 0; i < path.size() - 1; i++) {
		
		/*
			subdividing straight line segments:
			get a unit vector in the direction from the start point to the finish point.
			place a waypoint at each specified distance using the unit vector.
		*/
		
		nexPoint = *path.at(i + 1);

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
			double angle2 = atan2(nexPoint.y - path.at(i + 2)->y, nexPoint.x - path.at(i + 2)->x);
			
			// positive is clockwise. turn angle from -pi to pi
			double turnAngle = (angle1 - angle2);
			if (turnAngle<0) {
				turnAngle += 2 * PI;
			}
			if (turnAngle > PI) {
				turnAngle -= 2 * PI;
			}

			if (abs(turnAngle) <= simpleTurnMaxAngleRad && i > 0) {// conduct a simple turn (i > 0 because we dont want to do it on the first turn to alight the quad bike)

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
					Log::i << finalTurnAngle * 180 / PI;
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

	clearPath();

	for (unsigned int i = 0; i < subdividedPath.size(); i++) {
		addPointBasic(subdividedPath.at(i));
		//Log::i << subdividedPath.at(i).x << ", " << subdividedPath.at(i).y << endl;
	}
	subdividedPath.clear();

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
		Point(26.7, -1.46),
		Point(60.5, -0.09),
		Point(77.3, -0.44),
		Point(95.6, -0.37),
		Point(113.9, -0.11),
		Point(130.8, -0.62),
		Point(180, 1.18)};

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

void SimpleNavigator::loop() {
	endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> seconds = endTime - startTime;

	if (path.size() <= 0)
		return;

	while (true) {
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}

	
}

