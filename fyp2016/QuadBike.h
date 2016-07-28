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

	void setThrottle(int t);
	void setSteerAng(int s);
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
	double velocity = 0;
	double heading = 0; // heading in degrees clockwise from vertical axis (+y).
	double steerAngle = 0;
	double requestedSteerAngle = 0;
};