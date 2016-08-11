#pragma once

#include "Point.h"
// @ Harry: NO. this is entirely why i wrote the Log class.
// please use it. its so simple that there is no reason to ever use cout directly.
//#include <iostream>
#include <cmath>
#include <string>

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
	double getIdleSpeed();
	std::string getState();

	void setThrottlePercentage(double percent);
	void setSteerAng(double s);
	void setBrake(bool b);
	bool setGear(int g);
	void setState(std::string s);

	double const width = 1.18;
	double const length = 1.86;
	double const wheelBase = 1.28;
	double const overHang = (length - wheelBase) / 2;
	double const wheelRadii = 0.3;
	double const wheelWidth = 0.25;
	double const maxSteerAngle = 25 * 3.141592 / 180;
	double const cruisesVelocity = 1.2;

	Point getRearL();
	Point getRearR();
	Point getFrontL();
	Point getFrontR();
	Point getRearC();
	Point getLWheel();
	Point getRWheel();
	Point getSensorTopLeft();

	void update();

private:
	Point location;
	double velocity;
	double heading; // heading in degrees clockwise from vertical axis (+y).
	double steerAngle;
	double requestedSteerAngle;
	double throttle;
	double throttleSpeed;
	double maxAcc;
	bool brakes;
	int gear;
	std::string state;
};