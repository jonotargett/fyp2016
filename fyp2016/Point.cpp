
#include "Point.h"
#include <cmath>


Point::Point()
{
	x = 0;
	y = 0;
}

Point::Point(double X, double Y) {
	x = X;
	y = Y;
}

Point::~Point()
{
}

void Point::normalise()
{
	double length = sqrt(x*x + y*y);
	x /= length;
	y /= length;
}

Point Point::operator+(const Point &)
{
	//TODO
	return Point();
}

Point Point::operator-(const Point &)
{
	//TODO
	return Point();
}

Point Point::operator*(double)
{
	//TOOD
	return Point();
}

Point Point::operator/(double)
{
	//TODO
	return Point();
}

Point Point::operator%(const Point &)
{
	//TODO
	return Point();
}
