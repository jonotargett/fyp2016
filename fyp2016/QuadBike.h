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
	bool getBrakes();
	int getGear();

	void setThrottlePercentage(double percent);
	void setSteerAng(double s);
	void setBrake(bool b);
	bool setGear(int g);

	double const width = 1.18;
	double const length = 1.86;
	double const wheelBase = 1.28;
	double const overHang = (length - wheelBase) / 2;
	double const wheelRadii = 0.3;
	double const wheelWidth = 0.25;
	double const maxSteerAngle = 22 * 3.141592 / 180;

	Point getRearL();
	Point getRearR();
	Point getFrontL();
	Point getFrontR();
	Point getRearC();
	Point getLWheel();
	Point getRWheel();

	void update();

private:
	Point location;
	double velocity;
	double heading; // heading in degrees clockwise from vertical axis (+y).
	double steerAngle;
	double requestedSteerAngle;
	double throttle;
	double throttleSpeed;
	double acceleration;
	bool brakes;
	int gear;
};