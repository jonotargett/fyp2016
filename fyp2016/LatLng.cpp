#include "LatLng.h"



LatLng::LatLng()
{
	latitude = 0.0;
	longitude = 0.0;
}

LatLng::LatLng(double a, double o) : latitude(a), longitude(o)
{

}


LatLng::~LatLng()
{
}


Point LatLng::relativeFrom(LatLng ll) {
	Point p; // = Point((ll.latitude - latitude) * 100000, (ll.longitude - longitude) * 100000);

	double r_earth = 6378000.0;

	p.y = (latitude - ll.latitude) * (M_PI / 180.0) * r_earth;
	p.x = (longitude - ll.longitude) * (M_PI / 180.0) * r_earth * cos(ll.latitude * M_PI / 180);

	Log::d << p.x << " / " << p.y << endl;

	return p;
}
/**

/*

Point LatLng::relativeFrom(LatLng ll) {

	Point p = Point(0, 0);

	double lat1 = latitude;
	double lon1 = longitude;
	double lat2 = ll.latitude;
	double lon2 = ll.longitude;

	double a = 6378137, b = 6356752.314245, f = 1 / 298.257223563; // WGS-84 ellipsoid params
	double L = (lon2 - lon1) * (M_PI / 180.0);
	double U1 = atan((1 - f) * tan((lat1) * (M_PI / 180.0)));
	double U2 = atan((1 - f) * tan((lat2) * (M_PI / 180.0)));
	double sinU1 = sin(U1), cosU1 = cos(U1);
	double sinU2 = sin(U2), cosU2 = cos(U2);

	double sinLambda, cosLambda, sinSigma, cosSigma, sigma, sinAlpha, cosSqAlpha, cos2SigmaM;
	double lambda = L, lambdaP;
	unsigned int iterLimit = 100;
	do {
		sinLambda = sin(lambda);
		cosLambda = cos(lambda);
		sinSigma = sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda)
			+ (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) * (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));
		
		if (sinSigma == 0)
			return p; // co-incident points

		cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
		sigma = atan2(sinSigma, cosSigma);
		sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
		cosSqAlpha = 1 - sinAlpha * sinAlpha;

		if (cosSqAlpha == 0) {	// check for divide by zero error
			cos2SigmaM = 0; // equatorial line: cosSqAlpha=0 (§6)
		} else {
			cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;
		}
		
		double C = f / 16 * cosSqAlpha * (4 + f * (4 - 3 * cosSqAlpha));
		lambdaP = lambda;
		lambda = L + (1 - C) * f * sinAlpha
			* (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
	} while (abs(lambda - lambdaP) > 1e-12 && --iterLimit > 0);

	if (iterLimit == 0)
		return p; // formula failed to converge

	double uSq = cosSqAlpha * (a * a - b * b) / (b * b);
	double A = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
	double B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));
	double deltaSigma = B
		* sinSigma
		* (cos2SigmaM + B
			/ 4
			* (cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM) - B / 6 * cos2SigmaM
				* (-3 + 4 * sinSigma * sinSigma) * (-3 + 4 * cos2SigmaM * cos2SigmaM)));
	
	double dist = b * A * (sigma - deltaSigma);
	double a_2 = atan(cosU2*sin(lambda) / (-sinU2*cosU2) + cosU1*sinU2*cos(lambda));

	p.x = dist * cos(a_2);
	p.y = dist * sin(a_2);
	return p;
}
/**/


/**
* Calculates geodetic distance between two points specified by latitude/longitude using Vincenty inverse formula
* for ellipsoids
*
* @param lat1
*            first point latitude in decimal degrees
* @param lon1
*            first point longitude in decimal degrees
* @param lat2
*            second point latitude in decimal degrees
* @param lon2
*            second point longitude in decimal degrees
* @returns distance in meters between points with 5.10<sup>-4</sup> precision
* @see <a href="http://www.movable-type.co.uk/scripts/latlong-vincenty.html">Originally posted here</a>
*/
/*
public static double distVincenty(double lat1, double lon1, double lat2, double lon2) {
	double a = 6378137, b = 6356752.314245, f = 1 / 298.257223563; // WGS-84 ellipsoid params
	double L = Math.toRadians(lon2 - lon1);
	double U1 = Math.atan((1 - f) * Math.tan(Math.toRadians(lat1)));
	double U2 = Math.atan((1 - f) * Math.tan(Math.toRadians(lat2)));
	double sinU1 = Math.sin(U1), cosU1 = Math.cos(U1);
	double sinU2 = Math.sin(U2), cosU2 = Math.cos(U2);

	double sinLambda, cosLambda, sinSigma, cosSigma, sigma, sinAlpha, cosSqAlpha, cos2SigmaM;
	double lambda = L, lambdaP, iterLimit = 100;
	do {
		sinLambda = Math.sin(lambda);
		cosLambda = Math.cos(lambda);
		sinSigma = Math.sqrt((cosU2 * sinLambda) * (cosU2 * sinLambda)
			+ (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) * (cosU1 * sinU2 - sinU1 * cosU2 * cosLambda));
		if (sinSigma == 0)
			return 0; // co-incident points
		cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
		sigma = Math.atan2(sinSigma, cosSigma);
		sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
		cosSqAlpha = 1 - sinAlpha * sinAlpha;
		cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;
		if (Double.isNaN(cos2SigmaM))
			cos2SigmaM = 0; // equatorial line: cosSqAlpha=0 (§6)
		double C = f / 16 * cosSqAlpha * (4 + f * (4 - 3 * cosSqAlpha));
		lambdaP = lambda;
		lambda = L + (1 - C) * f * sinAlpha
			* (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
	} while (Math.abs(lambda - lambdaP) > 1e-12 && --iterLimit > 0);

	if (iterLimit == 0)
		return Double.NaN; // formula failed to converge

	double uSq = cosSqAlpha * (a * a - b * b) / (b * b);
	double A = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
	double B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));
	double deltaSigma = B
		* sinSigma
		* (cos2SigmaM + B
			/ 4
			* (cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM) - B / 6 * cos2SigmaM
				* (-3 + 4 * sinSigma * sinSigma) * (-3 + 4 * cos2SigmaM * cos2SigmaM)));
	double dist = b * A * (sigma - deltaSigma);

	return dist;
}

*/