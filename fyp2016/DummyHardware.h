#pragma once
#include "HardwareInterface.h"

#include <algorithm>
#include <ctime>

#include "Log.h"
#include "Matrix.h"

// want our updateloop to run at 60hz.
#define REFRESH_RATE 60
#ifndef PI
#define PI 3.1415926535
#endif


class DummyHardware :
	public HardwareInterface
{
public:
	DummyHardware();
	~DummyHardware();

	bool initialise();
	bool updateLoop();
	
	void setDesiredVelocity(double);
	void setDesiredSteeringAngle(double);
	void setDesiredThrottlePercentage(double);
	void setDesiredBrake(double);
	void setDesiredGear(Gear);
	void emergencyStop();
	
	Point getRealPosition();
	double getRealAbsoluteHeading();
	double getRealVelocity();
	double getRealSteeringAngle();
	double getRealThrottlePercentage();
	int getRealGear();
	double getRealBrakePercentage();

	Point getKinematicPosition();
	Point getAccelerometerPosition();
	Point getGPSPosition();
	Point getKalmanPosition();

private:
	void update(double);
	void updateActuators(double);
	void updateVelocityActuators();
	double random();
	void setUpKalmanMatrices();

	Point realPosition;
	double realAbsoluteHeading;
	double realVelocity;
	double realSteeringAngle;
	double realThrottlePercentage;
	Gear realGear;
	double realBrakePercentage;

	Point kinematicPosition;
	double kinematicHeading;
	Point accelerometerPosition;
	Point gpsPosition;
	Point oldgpsPosition;
	double timeSinceLastGpsUpdate;

	double desiredSteeringAngle;
	double desiredVelocity;
	double desiredThrottlePercentage;
	Gear desiredGear;
	double desiredBrakePercentage;

	double gearTimer;

	//kalman filter matrices:
	// prediction step:
	Matrix<double> mu;					// state space
	Matrix<double> sigma;				// uncertainty of motion, covariance matrix
	Matrix<double> g;					// state update function

	// correction step:
	Matrix<double> K;					// Kalman Gain
	Matrix<double> G;					// Jacobian of g
	Matrix<double> R;					// motion noise from g
	Matrix<double> h;					// observation function, h
	Matrix<double> H;					// Jacobian of h
	Matrix<double> z;					// observation
	Matrix<double> I;					// identity matrix
	Matrix<double> Q;					// uncertainty of sensor observation

	Matrix<double> A;


	double velocityChangeRate;			// metres/second^2
	double frictionalDecayRate;			// %/second??
	double brakingAcceleration;			// m/s/s at 100% brake. interpolate inbetween

	double positionPrecision;			// meters of spread each side of real value
	double driftSpeed;					// drift speed of real position
	double headingAccuracy;				// radians of spread each side of real value
	double velocityAccuracy;			// m/s of spread each side of real value
	double steeringAccuracy;			// radians of spread each side of real value
	double brakeAccuracy;				// percent of spread each side of real value
	double throttleAccuracy;			// percent of spread each side of real value
	double gpsAccuracy;					// meters spread each side of real value

};



