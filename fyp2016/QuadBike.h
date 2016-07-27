#pragma once

#include "Point.h"
#include <math.h>

class QuadBike
{
public:
	QuadBike();
	~QuadBike();

	Point getLocation();
	double getHeading();

	double const width = 1.18;
	double const length = 1.86;
	double const wheelBase = 1.28;
	double const overHang = (length - wheelBase) / 2;

	Point getRearL();
	Point getRearR();
	Point getFrontL();
	Point getFrontR();
	

private:
	Point location = Point(1.33, 1.33);
	
	double velocity = 0;
	double heading = 0; // heading in degrees clockwise from vertical axis (+y).
};