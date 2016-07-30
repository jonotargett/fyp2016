#pragma once

#include "Point.h"
#include <cmath>

class QuadBike
{
public:
	QuadBike();
	~QuadBike();

	Point getLocation();
	double getHeading();
	double getVelocity();
	double getSteerAng();
	double getThrottle();

	void setThrottle(double percent);
	void setSteerAng(double s);
	void setBrake(bool b);
	void setGear(int g);

	double const width = 1.18;
	double const length = 1.86;
	double const wheelBase = 1.28;
	double const overHang = (length - wheelBase) / 2;
	double const wheelRadius = 0.2;
	double const maxSteerAngle = 23 * 3.141592 / 180;

	Point getRearL();
	Point getRearR();
	Point getFrontL();
	Point getFrontR();
	Point getRearC();

	void update();
	

private:
	Point location = Point(1.33, 1.33);
	double velocity;
	double heading; // heading in degrees clockwise from vertical axis (+y).
	double steerAngle;
	double requestedSteerAngle;
	double throttle;
	double throttleSpeed;
	double acceleration;
};