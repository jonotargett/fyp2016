#include "QuadBike.h"
#include <iostream>

QuadBike::QuadBike()
{
	location.x = 0;
	location.y = 0;
	velocity = 1;
	heading = 190 * 3.141592 / 180;
}
QuadBike::~QuadBike()
{

}

void QuadBike::update() {

	if (steerAngle > 20 * 3.1415 / 180) steerAngle = 20 * 3.1415 / 180;
	if (steerAngle < -20 * 3.1415 / 180) steerAngle = -20 * 3.1415 / 180;

	// loop is runnin approx every 0.014 seconds (70 fps)
	//TODO: implement a proper loop so fps is actually accurate and not a guess
	double fps = 70;
	double distanceTravelled = velocity * 1 / fps;
	double distanceForward = 0;
	double distanceRight = 0;
	double angleTurned = 0;

	if (steerAngle == 0) {
		distanceForward = distanceTravelled;
	}
	else {
		double turnRadius = wheelBase / tan(steerAngle);
		angleTurned = distanceTravelled / turnRadius;
		distanceForward = turnRadius * sin(angleTurned);
		distanceRight = turnRadius - turnRadius * cos(angleTurned);
	}

	location.x += distanceForward * sin(heading) + distanceRight * cos(heading);
	location.y += distanceForward * cos(heading) - distanceRight * sin(heading);
	heading += angleTurned;
}

Point QuadBike::getLocation() {
	return location;
}

double QuadBike::getHeading() {
	return heading;
}

double QuadBike::getVelocity() {
	return velocity;
}

Point QuadBike::getRearL() {
	Point rearLeft;
	rearLeft.x = -(overHang + wheelBase)*sin(getHeading()) - (width / 2) * cos(getHeading());
	rearLeft.y = -(overHang + wheelBase)*cos(getHeading()) + (width / 2) * sin(getHeading());
	return rearLeft;
}
Point QuadBike::getRearR() {
	Point rearRight;
	rearRight.x = -(overHang + wheelBase)*sin(getHeading()) + (width / 2) * cos(getHeading());
	rearRight.y = -(overHang + wheelBase)*cos(getHeading()) - (width / 2) * sin(getHeading());
	return rearRight;
}
Point QuadBike::getFrontL() {
	Point frontLeft;
	frontLeft.x = overHang * sin(getHeading()) - (width / 2) * cos(getHeading());
	frontLeft.y = overHang * cos(getHeading()) + (width / 2) * sin(getHeading());
	return frontLeft;
}
Point QuadBike::getFrontR() {
	Point frontRight;
	frontRight.x = overHang * sin(getHeading()) + (width / 2) * cos(getHeading());
	frontRight.y = overHang * cos(getHeading()) - (width / 2) * sin(getHeading());
	return frontRight;
}