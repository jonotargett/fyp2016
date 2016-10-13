#include "SimpleNavigator.h"



SimpleNavigator::SimpleNavigator()
{
	//isForwards = true;
	travelPathForwards = true;
	currentPathPoint = 0;
	navState = NAV_CRUISE;
	nextTurnPoint = 0;
	distanceToTurn = 0;
	converging = true;

	noTurnMaxRads = 15 * PI / 180;
	simpleTurnMaxAngleRad = 40 * PI / 180;
	distanceBetweenWaypoints = 0.5;			// meters
	distanceBetweenTurnWaypoints = 0.2;
	minTurnRadius = 3.25;		// min turn radius of the quad bike

}


SimpleNavigator::~SimpleNavigator()
{
}


bool SimpleNavigator::initialise() {
	// doing this because i have no idea how to poperly initialise the vector
	addPoint(Point(0, 0));
	pathNavigationCompleted = true;

	Log::d << "Navigator initialised." << std::endl;
	return true;
	
}

void SimpleNavigator::clearPath() {
	path.clear();
}

void SimpleNavigator::clearSubdividedPath() {
	subdividedPath.clear();
}

void SimpleNavigator::setPath(std::vector<Point>) {

}

bool SimpleNavigator::isConverging() {
	return converging;
}

bool SimpleNavigator::updatePoint(Point position, float heading, float velocity) {

	if (!isNextPoint() && !pathNavigationCompleted) {
		Log::i << "No next point -> Path navigation complete." << endl;
		pathNavigationCompleted = true;
		return false;
	}

	double lookAheadDistance = 1.5;
	double turnTolerance = 0.2;
	int increment = 0;
	increment = (travelPathForwards) ? 1 : -1;

	// when incrementing the point, the distnace has to get bigger or smaller,
	// if the distance gets bigger, keep incremening until distance is bigger
	// than look ahead distance
	// if the distance gets smaller, we have a turn inbound. stop incrememting!

	double distanceNow = position.getDistanceTo(subdividedPath.at(currentPathPoint));
	double distanceNext = position.getDistanceTo(subdividedPath.at(currentPathPoint + increment));

	// while we are cruising, and less than the look ahead distnace
	while (navState == NAV_CRUISE && distanceNow < lookAheadDistance) {
		if (!isNextPoint()) {
			// have already incremented pathPoint by now
			return true;
		}
		currentPathPoint += increment;
		distanceNow = position.getDistanceTo(subdividedPath.at(currentPathPoint));
		distanceNext = position.getDistanceTo(subdividedPath.at(currentPathPoint + increment));

		if (distanceNow > distanceNext) {
			// we have an n-point turn coming up
			navState = NAV_TURNINBOUND;
			distanceToTurn = distanceNow;
			turnPoint = currentPathPoint;
			break;
		}
	}

	// are we converging on the turn?
	if (navState == NAV_CRUISE) {
		converging = true;
	}
	if (navState == NAV_TURNINBOUND) {
		if (distanceNow > distanceToTurn) {
			converging = false;
		}
		else {
			distanceToTurn = distanceNow;
		}
	}

	// turn stuff
	// we know where the first turn point is at this stage, turnPoint
	// is there another turn point after this?:
	// if we are within turntolerance or are no longer converging, we have reached the turn point
	if ((distanceNow < turnTolerance || converging == false) && navState == NAV_TURNINBOUND && velocity == 0) {
		converging = true;
		//isForwards = !isForwards;
		navState = NAV_CRUISE;

		// but do we have another turn???
		double nowTurnDistance = 0;
		double nextTurnDistance = 0;
		nextTurnPoint = turnPoint;

		do {
			nowTurnDistance = subdividedPath.at(turnPoint).getDistanceTo(subdividedPath.at(currentPathPoint));
			nextTurnDistance = subdividedPath.at(turnPoint).getDistanceTo(subdividedPath.at(currentPathPoint + increment));
			if (nowTurnDistance > nextTurnDistance) {

				// we have another turn coming up
				navState = NAV_TURNINBOUND;
				turnPoint = currentPathPoint;
				distanceToTurn = position.getDistanceTo(subdividedPath.at(currentPathPoint));
				break;
			}
			if (!isNextPoint()) {
				// there is no next turn point because theres no next point
				break;
			}
			currentPathPoint += increment;
		} while (nowTurnDistance < lookAheadDistance);
	}

}

Point SimpleNavigator::getPoint() {
	if (currentPathPoint < subdividedPath.size()) {
		return subdividedPath.at(currentPathPoint);
	}
	else {
		return Point(NULL, NULL);
	}
	
}

/*bool SimpleNavigator::getIsForwards() {
	return isForwards;
}*/

bool SimpleNavigator::isNextPoint() {
	if (travelPathForwards) {
		if (currentPathPoint + 1 >= subdividedPath.size() - 1) {
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
void SimpleNavigator::addPoint(Point p) {
	path.push_back(p);
}

void SimpleNavigator::addPoint(LatLng ll) {
	Point np = ll.relativeFrom(baseLoc);
	path.push_back(np);
}


bool SimpleNavigator::startPath() {

	updater = new std::thread(&SimpleNavigator::loop, this);
	return true;
}

std::vector<Point> SimpleNavigator::getSubdividedPath() {
	return subdividedPath;
}

void SimpleNavigator::setBaseLocation(LatLng ll) {
	baseLoc = ll;
}

/*
subdivide: takes user defined waypoints and subdivides them with waypoints
spaced apart as defined in variables at start of function.  1 unit represents
1m. function loops through each line segment. for each iteration the line
segment is subdivided and waypoints for a turn into the next line segment is
calculated and added.
*/
bool SimpleNavigator::subdivide(Point quadPosition, float heading) {
	
	//subdividedPath = path;
	//return true;

	// reset the current path point for nav purposees
	currentPathPoint = 0;
	pathNavigationCompleted = false;
	
	// need to connect the quadbike to the path, if it isnt already.
	// add a line which extends from the front of hte quad bike a small distance,
	// then connect that line to the first point in the path.
	// TODO

	// TODO(Harry) make sure points arent too close together, if they are remove one.

	clearSubdividedPath();

	
	/*
	for each line segment (each line between two 'ultimate' waypoints)
	subdivide the straight line segment:
	get a unit vector in the direction from the start point to the finish point.
	place a waypoint at each specified distance using the unit vector.
	the step for each line segment excludes the very first point of the line segment
	and includes the very last point of hte line segment.
	*/
	Point curPoint = path.at(0);
	Point nexPoint = path.at(1);

	subdividedPath.push_back(path.at(0));

	for (unsigned int i = 0; i < path.size() - 1; i++) {
		nexPoint = path.at(i+1);

		Point directionVector = Point(nexPoint.x - curPoint.x, nexPoint.y - curPoint.y);
		directionVector.normalise();

		// FactorXY used for determining whether intermediate point is still within boundary points.
		double FactorX = (directionVector.x != 0) ? directionVector.x / abs(directionVector.x) : 0;
		double FactorY = (directionVector.y != 0) ? directionVector.y / abs(directionVector.y) : 0;

		// while our intermediate point is still between the two 'ultimate' waypoints
		// intermediate is calculated based on curPoint to remove accumulative error
		int index = 1;
		Point intermediate = Point();						// intermediate holds the current intermediate waypoint
		intermediate.x = curPoint.x + index * (directionVector.x * distanceBetweenWaypoints);
		intermediate.y = curPoint.y + index * (directionVector.y * distanceBetweenWaypoints);
		while (intermediate.x * FactorX <= nexPoint.x * FactorX && intermediate.y * FactorY <= nexPoint.y * FactorY) {
			subdividedPath.push_back(Point(intermediate.x, intermediate.y));
			index++;
			intermediate.x = curPoint.x + index * (directionVector.x * distanceBetweenWaypoints);
			intermediate.y = curPoint.y + index * (directionVector.y * distanceBetweenWaypoints);
		}
		// add the last point we might have missed in the while loop
		if (subdividedPath.at(subdividedPath.size() - 1).x != nexPoint.x || subdividedPath.at(subdividedPath.size() - 1).y != nexPoint.y) {
			subdividedPath.push_back(nexPoint);
		}

		/*
		waypoints for turn subdivision calculated below
		turn orients the quadbike such that its heading matches the heading
		of the next line segment. The main for loop (iterating on i) can
		then proceed.
		uses a vector from the centre of the turn circle to the quadbike to
		calculate waypoints. the vector is incrementally rotated and a
		waypoint added at each step.
		*/

		// if there is a next line segment to line up with (if we need to turn):
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


			if (abs(turnAngle) <= noTurnMaxRads) {
				// we dont need to do a turn, just start subdividing hte next line.
			}
			else if (abs(turnAngle) <= simpleTurnMaxAngleRad) {
				// conduct a simple turn
				if (!simpleTurn(directionVector, turnAngle, i + 1, curPoint)) {
					nPointTurn(directionVector, turnAngle, i + 1);
				}
			}
			else {
				// conduct N-Point turn
				nPointTurn(directionVector, turnAngle, i + 1);
			}

		}
		curPoint = subdividedPath.at(subdividedPath.size() - 1);	// start the next line segment from the very last point in the path.
	}
	/*for (unsigned int i = 0; i < subdividedPath.size(); i++) {
		Log::i << subdividedPath.at(i).x << ", " << subdividedPath.at(i).y << endl;
	}*/

	Log::d << "Path subdivision completed" << endl;
	return true;
}

bool SimpleNavigator::simpleTurn(Point heading, double turnAngle, int turnIndexPoint, Point previousLinearPoint) {
	// how much room either side of 'index' that we need to do the turn
	// and check to see if we have that much room
	double d = abs(minTurnRadius * tan(turnAngle / 2));
	Point prePoint = previousLinearPoint;
	Point indexPoint = path.at(turnIndexPoint);
	Point endPoint = path.at(turnIndexPoint + 1);
	if (prePoint.getDistanceTo(indexPoint) <= d || indexPoint.getDistanceTo(endPoint) <= d) {
		return false;
	}
	
	// remove the last few points from the straight line segment up to a distance d from the turn waypoint
	double distanceFromRemoved = indexPoint.getDistanceTo(subdividedPath.at(subdividedPath.size() - 1));
	while (distanceFromRemoved <= d) {
		subdividedPath.erase(subdividedPath.end() - 1);
		distanceFromRemoved = indexPoint.getDistanceTo(subdividedPath.at(subdividedPath.size() - 1));
	}

	// start the turn from a distance d from indpoint
	Point startTurn = indexPoint - (heading * d);

	// turnVector holds the vector from the centre of the turning circle
	// to the quad bike. needs some manipulation before we have the
	// correct vector.
	// get direction vector from initial point to final point of line segment
	Point turnVector = heading;
	turnVector.normalise();
	turnVector.x *= minTurnRadius;
	turnVector.y *= minTurnRadius;

	int turnFactor = (turnAngle >= 0) ? -1 : 1;		// factor to multiply angles and such. if a right turn = -1, if left = 1
	double tempX = turnVector.x;					// first turn arc, a RIGHT turn arc has negative x, a LEFT turn arc has negative y
	turnVector.x = turnVector.y * turnFactor;
	turnVector.y = -tempX * turnFactor;

	// centre of the turning circle
	Point centre = startTurn - turnVector;

	// k increments the angle and places multiple waypoints for each of the turns.
	// increment holds the change in angle needed
	double increment = distanceBetweenTurnWaypoints / minTurnRadius;
	for (double k = increment; k < abs(turnAngle); k += increment) {
		double newVecX = cos(k * turnFactor) * turnVector.x - sin(k * turnFactor) * turnVector.y;
		double newVecY = sin(k * turnFactor) * turnVector.x + cos(k * turnFactor) * turnVector.y;
		if (k > abs(turnAngle)) {
			break;
		}
		Point pp = Point(centre.x + newVecX, centre.y + newVecY);
		subdividedPath.push_back(pp);
	}

	// add the last point in the turn we would have missed in the loop
	double k = abs(turnAngle);
	double newVecX = cos(k * turnFactor) * turnVector.x - sin(k * turnFactor) * turnVector.y;
	double newVecY = sin(k * turnFactor) * turnVector.x + cos(k * turnFactor) * turnVector.y;
	Point pp = Point(centre.x + newVecX, centre.y + newVecY);
	subdividedPath.push_back(pp);

	return true;
}
bool SimpleNavigator::nPointTurn(Point heading, double turnAngle, int turnIndexPoint) {
	// excludes the very first point and includes the very last point (of each sub turn)
	heading.normalise();
	//delta Y stuff here
	double deltaY = getDeltaY(abs(turnAngle));
	if (deltaY > 0.3) deltaY = 0.3;
	Point turnPoint;
	turnPoint = path.at(turnIndexPoint);
	turnPoint.x -= heading.x * deltaY;
	turnPoint.y -= heading.y * deltaY;
	subdividedPath.push_back(turnPoint);


	// heading of quad bike at each point of an N-point turn (from google sketchup)
	// currently based on a max turn angle of 21.5 degrees, 
	// -> minTurnRadius = 3.25m
	std::vector<double> turnAngleListDegrees = {
		0, 26.7, 60.5, 77.3, 95.6, 113.9, 130.8, 180 };

	// convert turnAngleListDegrees to radians
	std::vector<double> turnAngleListRads;
	for (unsigned int m = 0; m < turnAngleListDegrees.size(); m++) {
		turnAngleListRads.push_back(turnAngleListDegrees.at(m) * PI / 180);
	}

	// turnVector holds the vector from the centre of the turning circle
	// to the quad bike. needs some manipulation before we have the
	// correct vector.
	// get direction vector from initial point to final point of line segment

	Point turnVector = heading;
	turnVector.normalise();
	turnVector.x *= minTurnRadius;
	turnVector.y *= minTurnRadius;

	int turnFactor = (turnAngle >= 0) ? 1 : -1;// factor to multiply angles and such. if a right turn = 1, if left = -1
	double tempX = turnVector.x;// first turn arc, a RIGHT turn arc has negative x, a LEFT turn arc has negative y
	turnVector.x = turnVector.y * turnFactor;
	turnVector.y = -tempX * turnFactor;

	Point centre = Point(turnPoint.x - turnVector.x, turnPoint.y - turnVector.y);	// centre of the turning circle

	bool hasReachedCorrectAngle = false;

	// j cycles through each of the turns in the N-point turn
	for (int j = 0; j < 7; j++) {

		// k increments the angle and places multiple waypoints for each of the turns.
		// increment holds the change in angle needed
		double increment = distanceBetweenTurnWaypoints / minTurnRadius;
		for (double k = increment; k < (turnAngleListRads.at(j + 1) - turnAngleListRads.at(j)); k += increment) {
			double newVecX = cos(k * turnFactor) * turnVector.x + sin(k * turnFactor) * turnVector.y;
			double newVecY = -sin(k * turnFactor) * turnVector.x + cos(k * turnFactor) * turnVector.y;

			if (k + turnAngleListRads.at(j) >= turnAngle * turnFactor) {
				hasReachedCorrectAngle = true;
				break;
			}

			Point pp = Point(centre.x + newVecX, centre.y + newVecY);
			subdividedPath.push_back(pp);
		}
		if (hasReachedCorrectAngle) {
			break;
		}
		//adding the last point that the for loop would have missed.
		double finalTurnAngle = (turnAngleListRads.at(j + 1) - turnAngleListRads.at(j));
		double newVecX = cos(finalTurnAngle * turnFactor) * turnVector.x + sin(finalTurnAngle * turnFactor) * turnVector.y;
		double newVecY = -sin(finalTurnAngle * turnFactor) * turnVector.x + cos(finalTurnAngle * turnFactor) * turnVector.y;
		Point pp = Point(centre.x + newVecX, centre.y + newVecY);
		subdividedPath.push_back(pp);

		if (finalTurnAngle + turnAngleListRads.at(j) >= turnAngle * turnFactor) {
			hasReachedCorrectAngle = true;
			break;
		}

		// set up direction vector for the next arc in the n-point turn
		double rotationAngle = turnAngleListRads.at(j + 1) - turnAngleListRads.at(j);
		double tempX = cos(rotationAngle * turnFactor) * turnVector.x + sin(rotationAngle * turnFactor) * turnVector.y;
		turnVector.y = -sin(rotationAngle * turnFactor) * turnVector.x + cos(rotationAngle * turnFactor) * turnVector.y;
		turnVector.x = tempX;

		//then flip it so we turn the other way
		turnVector.y *= -1;
		turnVector.x *= -1;

		centre.x -= 2 * turnVector.x;
		centre.y -= 2 * turnVector.y;
	}

	return true;
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

