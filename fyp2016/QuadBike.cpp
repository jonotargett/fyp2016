#include "QuadBike.h"

QuadBike::QuadBike()
{
	location.x = 0;
	location.y = 0;
	heading = 30 * 3.141592 / 180;
}
QuadBike::~QuadBike()
{

}

Point QuadBike::getLocation() {
	return location;
}

double QuadBike::getHeading() {
	return heading;
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