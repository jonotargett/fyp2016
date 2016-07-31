#include "QuadBike.h"
#include <iostream>

QuadBike::QuadBike()
{
	location.x = -0.1;
	location.y = 0;
	velocity = 0;
	heading = 0 * 3.141592 / 180;
	throttle = 0;
	steerAngle = 0;
	requestedSteerAngle = 0;
	throttleSpeed = 0;
	acceleration = 2; //m/s/s
	brakes = true;
	gear = 0; // -1 reverse, 0 neutral, 1 drive
}
QuadBike::~QuadBike()
{

}

void QuadBike::update() {

	// loop is runnin approx every 0.014 seconds (70 fps)
	//TODO: implement a proper loop so fps is actually accurate and not a guess

	double fps = 70;
	double distanceTravelled = velocity * 1 / fps;
	double distanceForward = 0;
	double distanceRight = 0;
	double angleTurned = 0;

	// steering stuff
	if (steerAngle < requestedSteerAngle) steerAngle += 0.01;
	if (steerAngle > requestedSteerAngle) steerAngle -= 0.01;
	if (steerAngle > maxSteerAngle) steerAngle = maxSteerAngle;
	if (steerAngle < -maxSteerAngle) steerAngle = -maxSteerAngle;

	// velocity/throttle stuff
	if (gear == -1) { // reverse
		//if vel is pretty close to desired value, set it to desired value
		if (abs(velocity - throttleSpeed * gear) < acceleration / fps) {
			velocity = throttleSpeed * gear;
		}
		else {
			if (velocity < throttleSpeed * gear) velocity += acceleration / 2 / fps;
			if (velocity > throttleSpeed * gear) velocity -= acceleration / fps;
		}
	}
	else if (gear == 1) { // forward
		//if vel is pretty close to desired value, set it to desired value
		if (abs(velocity - throttleSpeed) < acceleration / fps) {
			velocity = throttleSpeed;
			
		} else {
			if (velocity < throttleSpeed) velocity += acceleration / fps;
			if (velocity > throttleSpeed) velocity -= acceleration / 2 / fps;
		}
	}
	else if (gear == 0) { // neutral
		//if vel is pretty close to desired value, set it to desired value
		if (abs(velocity) < acceleration / 2 / fps) {
			velocity = 0;
		}
		else {
			if (velocity > 0) velocity -= acceleration / 2 / fps;
			if (velocity < 0) velocity += acceleration / 2 / fps;
		}
	}

	// brake stuff
	if (brakes) {
		//if vel is pretty close to desired value, set it to desired value
		if (abs(velocity) < acceleration / fps) {
			velocity = 0;
		}
		else {
			if (velocity > 0) velocity -= acceleration / fps;
			if (velocity < 0) velocity += acceleration / fps;
		}
	}

	// calculating next position based on steer angle and velocity
	if (steerAngle == 0) {
		distanceForward = distanceTravelled;
	}
	else {
		double turnRadius = wheelBase / tan(-steerAngle);
		angleTurned = distanceTravelled / turnRadius;
		distanceForward = turnRadius * sin(angleTurned);
		distanceRight = turnRadius - turnRadius * cos(angleTurned);
	}

	// updating position
	location.x += distanceForward * sin(heading) + distanceRight * cos(heading);
	location.y += distanceForward * cos(heading) - distanceRight * sin(heading);
	heading += angleTurned;
	if (heading > 3.14159265) heading -= 2 * 3.14159265;
	if (heading < -3.14159265) heading += 2 * 3.14159265;
}

void QuadBike::setThrottlePercentage(double percent) {
	// to mimic real life conditions, speed at 0% =~ 0.25 m/s, and speed at 100% = 15m/s.
	// speed will gradually approach corresponding throttle speed.
	// Throttle speed = 0.25 + 0.1475 * percent
	if (percent > 100) percent = 100;
	if (percent < 0) percent = 0;
	throttle = percent;
	throttleSpeed = 0.25 + 0.1475 * percent;
}
void QuadBike::setSteerAng(double s) {
	if (s > maxSteerAngle) s = maxSteerAngle;
	if (s < -maxSteerAngle) s = -maxSteerAngle;
	requestedSteerAngle = s;
}
void QuadBike::setBrake(bool b) {
	brakes = b;
}
bool QuadBike::setGear(int g) {
	// can go into neutral anytime
	// returns true if successful gear change, false otherwise
	if (g > 1) return false;
	if (g < -1) return false;
	if (velocity == 0) {
		gear = g;
		return true;
	}
	else {
		if (g == 0) {
			gear = g;
			return true;
		} else {
			return false;
		}
	}
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

double QuadBike::getSteerAng() {
	return steerAngle;
}

double QuadBike::getThrottle() {
	return throttle;
}

bool QuadBike::getBrakes() {
	return brakes;
}

int QuadBike::getGear() {
	return gear;
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
Point QuadBike::getRearC() {
	Point rearCenter;
	rearCenter.x = -(wheelBase)*sin(getHeading());
	rearCenter.y = -(wheelBase)*cos(getHeading());
	return rearCenter;
}

Point QuadBike::getRWheel() {
	Point rearRight;
	rearRight.x = -(wheelBase)*sin(getHeading()) + (width / 2 - wheelWidth / 2) * cos(getHeading()) - wheelWidth / 2;
	rearRight.y = -(wheelBase)*cos(getHeading()) - (width / 2 - wheelWidth / 2) * sin(getHeading()) + wheelRadii;
	return rearRight;
}

Point QuadBike::getLWheel() {
	Point rearLeft;
	rearLeft.x = -(wheelBase)*sin(getHeading()) - (width / 2 - wheelWidth / 2) * cos(getHeading()) - wheelWidth / 2;
	rearLeft.y = -(wheelBase)*cos(getHeading()) + (width / 2 - wheelWidth / 2) * sin(getHeading()) + wheelRadii;
	return rearLeft;
}