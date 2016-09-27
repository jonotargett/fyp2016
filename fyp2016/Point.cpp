
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

double Point::getDistanceTo(Point p) {
	double distance = sqrt(pow(x - p.x, 2) + pow(y - p.y, 2));
	return distance;
}

double Point::getLength() {
	return getDistanceTo(Point(0, 0));
}

void Point::normalise()
{
	double length = sqrt(x*x + y*y);
	x /= length;
	y /= length;
}

Point operator+(const Point& p1, const Point& p2)
{
	return Point(p1.x + p2.x, p1.y + p2.y);
}

Point operator-(const Point& p1, const Point& p2)
{
	return Point(p1.x - p2.x, p1.y - p2.y);
}

Point Point::operator*(double d)
{
	return Point(x * d, y * d);
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
