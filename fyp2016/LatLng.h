#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include "Point.h"
#include "Log.h"

struct LatLng
{
public:
	LatLng();
	LatLng(double, double);
	~LatLng();

	double latitude;
	double longitude;

	Point relativeFrom(LatLng ll);
};

