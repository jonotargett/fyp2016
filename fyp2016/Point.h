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

	//vector addition
	Point operator+(const Point&);
	Point operator-(const Point&);

	//piecewise multiplication/division
	Point operator*(double);
	Point operator/(double);

	//dot product
	Point operator%(const Point&);
};

