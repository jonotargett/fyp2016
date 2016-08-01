#pragma once
class Point
{
private:
public:
	Point();
	Point(double, double);
	~Point();

	double x;
	double y;

	//new vector length == 1
	void normalise();

	double getDistanceTo(Point p);

	//vector addition
	friend Point operator+(const Point& p1, const Point& p2);
	Point operator-(const Point&);

	//piecewise multiplication/division
	Point operator*(double);
	Point operator/(double);

	//dot product
	Point operator%(const Point&);
};

