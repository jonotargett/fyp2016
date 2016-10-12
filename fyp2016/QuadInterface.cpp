#include "QuadInterface.h"

/*

NOTE FOR HARRY: IF YOURE MERGING YOUR QUADBIKE CLASS;
THIS IS NOT THE PLACE TO DO IT. YOUR VIRTUAL QUADBIKE
BELONGS EITHER IN THE SAME FILE YOU MADE IT, BUT
REARRANGED TO IMPLEMENT THE HARDWARE INTERFACE SPECS,
OR MERGE WITH DUMMY HARDWARE.

THIS IS WHERE THE ACTUAL HARDWARE INTERFACE STUFF BELONGS.

*/

QuadInterface::QuadInterface()
{
	comPort = 1;
	connected = false;
	current = std::chrono::high_resolution_clock::now();
	lastThrottle = current;
	lastSteering = current;
	lastGear = current;
	lastBrake = current;

	desiredVelocity = 0;
}


QuadInterface::~QuadInterface()
{
}


bool QuadInterface::initialise() {

	bool comPortKnown = false;
	bool success = false;

#ifdef COM_PORT
	comPort = COM_PORT;
	comPortKnown = true;
#endif

	Log::d << "Establishing COM link to hardware controller..." << endl;

	if (!comPortKnown) {
		Log::d << "Searching for open COM connections..." << endl;

		while (comPort <= MAX_COM_SEARCH) {
			success = establishCOM(comPort);
			if (success)
				break;

			//just hang for a bit. if we hammer the serial ports,
			// Windows slows down opening/reopening and we'll miss the timeout
			// window. so just chill for a bit after a failed connect.
			std::this_thread::sleep_for(std::chrono::milliseconds(SEARCH_INTERVAL));
			++comPort;
		}
		if(!success)
			return false;
	}
	else {
		success = establishCOM(comPort);
		if (!success) {
			Log::e << "Failed to establish communication on COM" << comPort << ". " << endl;
			return false;
		}
	}

	Log::i << "Connection established on COM" << comPort << endl;

	start();

	// leave the port open, normally. this was just here for debug purposes
	//serial.Close();
	connected = true;

	return true;
}


bool QuadInterface::establishCOM(int portnum) {
	bool success = false;

	// check if the port can be opened

	Log::d << "Open COM" << portnum << "...";
	success = (bool)serial.Open(portnum, BAUD_RATE);

	if (success) {
		Log::d << " opened" << endl;
	}
	else {
		Log::d << " failure" << endl;
		return false;
	}

	// if the port can be opened, check that it connects to a
	// responsive QuadBike.

	Packet p = Packet();

	p.packetID = ID_IDLE;
	p.length = 0;
	p.data = NULL;

	uint8_t* bytes = p.toBytes();

	// send the query packet
	
	startTime = std::chrono::high_resolution_clock::now();
	uint8_t* ret = new uint8_t[1];
	bool finished = false;
	collectingPacket = false;
	int count = 0;
	Packet* rp = NULL;

	count = serial.SendData((char*)bytes, p.getByteLength());
	if (count != p.getByteLength()) {
		Log::d << "Sending of handshake packet was interrupted." << endl;
		serial.Close();
		return false;
	}
	//Log::d << "Sent " << count << endl;

	// waiting for the return packet
	
	while (!finished) {
		endTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> seconds = endTime - startTime;

		if (seconds.count() * 1000 > COM_TIMEOUT) {
			Log::d << "Handshake not received within timeout." << endl;
			serial.Close();
			return false;
		}

		if (serial.ReadDataWaiting() > 0) {
			

			//read 1 byte at a time
			count = serial.ReadData(ret, 1);
			if (count != 1) {
				Log::d << "Invalid COM read error" << endl;
				serial.Close();
				return false;
			}
			else {

				//Log::d << "got something... 0x" << std::hex << ret[0] << std::dec << endl;

				if (ret[0] == ID_SOH) {
					collectingPacket = true;
				}
				else if (ret[0] == ID_ETB) {
					collectingPacket = false;
					rp = processPacket();
					finished = true;
					break;
				}
				else if (collectingPacket) {
					receivedBuffer.push(ret[0]);
				}
			}

		}
	}

	if (rp->packetID != ID_IDLE) {
		Log::d << "Invalid handshake response received" << endl;
		serial.Close();
		return false;
	}

	Log::d << "Handshake received in full" << endl;
	return true;
}


Packet* QuadInterface::processPacket() {
	Packet* p = new Packet();

	p->packetID = (ID)receivedBuffer.front();
	receivedBuffer.pop();
	p->length = receivedBuffer.front();
	receivedBuffer.pop();

	p->data = new float[p->length];

	if (receivedBuffer.size() == (p->length * 4)) {
		//Log::e << "completed packet" << endl;
	}
	else {
		Log::e << "Communications error: corrupted/invalid packet received" << endl;
		delete p;
		while (receivedBuffer.size() > 0) {
			receivedBuffer.pop();
		}
		return false;
	}

	for (int i = 0; i < p->length; i++) {
		float result;
		uint8_t b0 = receivedBuffer.front();
		receivedBuffer.pop();
		uint8_t b1 = receivedBuffer.front();
		receivedBuffer.pop();
		uint8_t b2 = receivedBuffer.front();
		receivedBuffer.pop();
		uint8_t b3 = receivedBuffer.front();
		receivedBuffer.pop();

		uint8_t byte_array[] = { b0, b1, b2, b3 };

		std::copy(reinterpret_cast<const uint8_t*>(&byte_array[0]),
			reinterpret_cast<const uint8_t*>(&byte_array[4]),
			reinterpret_cast<uint8_t*>(&result));

		p->data[i] = result;
	}

	//Log::i << "PACKET RECEIVED: " << (int)p->packetID << " / " << (int)p->length << endl;
	//Log::d << p->data[0] << "/" << p->data[1] << "/" << p->data[2] << "/" << p->data[3] << endl;

	return p;
}

void QuadInterface::setInitialQuadPosition(double longitude, double latitude) {
	double adjustedLong = longitude - 138.33;
	double adjustedLat = latitude + 35.1;
	setPosition(Point(longitude* 111312 * cos(latitude), latitude * 111312));
}



bool QuadInterface::updateLoop() {
	updateVelocityActuators();

	//check for communications
	int count = 0;
	uint8_t* ret = new uint8_t[1];
	Packet* rp = NULL;
	bool finished = false;

	if (serial.ReadDataWaiting() > 0) {
		//read 1 byte at a time
		count = serial.ReadData(ret, 1);
		if (count != 1) {
			Log::d << "Invalid COM read error" << endl;
			serial.Close();
			return false;
		}
		else {

			//Log::d << "got something... 0x" << std::hex << ret[0] << std::dec << endl;

			if (ret[0] == ID_SOH) {
				collectingPacket = true;
			}
			else if (ret[0] == ID_ETB) {
				collectingPacket = false;
				rp = processPacket();
				finished = true;
			}
			else if (collectingPacket) {
				receivedBuffer.push(ret[0]);
			}
		}
	}

	//if we just received a finished packet
	if (finished) {
		switch (rp->packetID) {
		case ID_QUAD_GEAR:
			if (rp->data[0] == 1) {
				setGear(GEAR_FORWARD);
			}
			else if (rp->data[0] == -1) {
				setGear(GEAR_REVERSE);
			}
			else {
				setGear(GEAR_NEUTRAL);
			}
			break;
		case ID_QUAD_BRAKE:
			setBrakePercentage((double)rp->data[0]);
			break;
		case ID_QUAD_THROTTLE:
			setThrottlePercentage((double)rp->data[0]);
			break;
		case ID_QUAD_STEERING:
			setSteeringAngle((double)rp->data[0]);
			break;
		case ID_QUAD_GPS: 
			{
				double gpsLong = rp->data[0];
				double gpsLat = rp->data[1];
				double adjustedLong = gpsLong - 138.33;
				double adjustedLat = gpsLat + 35.1;
				setGpsPosition(Point(adjustedLong * 111312 * cos(gpsLat), adjustedLat * 111312));
			}
			break;
		case ID_QUAD_IMU:
			setImuHeading(rp->data[0]);
			break;
		case ID_QUAD_SPEED:
			setVelocity(rp->data[0]);
			break;
		case ID_IDLE:
			//do something to check the timeout with the arduino
			break;
		default:
			// do nothing?
			break;
		}

	}

	return true;
}

void QuadInterface::setDesiredVelocity(double v) {
	desiredVelocity = v;
}

void QuadInterface::setDesiredSteeringAngle(double a) {
	if (!connected)
		return;

	current = std::chrono::high_resolution_clock::now();
	seconds = current - lastSteering;

	if (seconds.count() > 0.100) {
		Packet* p = new Packet();

		p->packetID = ID_SET_QUAD_STEERING;
		p->length = 1;
		p->data = new float[1];
		p->data[0] = (float)a;

		uint8_t* bytes = p->toBytes();

		serial.SendData((char*)bytes, p->getByteLength());

		lastSteering = current;
	}
}

void QuadInterface::setDesiredThrottlePercentage(double t) {
	if (!connected)
		return;

	current = std::chrono::high_resolution_clock::now();
	seconds = current - lastThrottle;

	if (seconds.count() > 0.100) {
		Packet* p = new Packet();

		p->packetID = ID_SET_QUAD_THROTTLE;
		p->length = 1;
		p->data = new float[1];
		p->data[0] = (float)t;

		uint8_t* bytes = p->toBytes();

		serial.SendData((char*)bytes, p->getByteLength());

		lastThrottle = current;
	}
}

void QuadInterface::setDesiredBrakePercentage(double b) {

	if (!connected)
		return;

	current = std::chrono::high_resolution_clock::now();
	seconds = current - lastBrake;

	if (seconds.count() > 0.100) {
		Packet* p = new Packet();

		p->packetID = ID_SET_QUAD_BRAKE;
		p->length = 1;
		p->data = new float[1];
		p->data[0] = (float)b;

		uint8_t* bytes = p->toBytes();

		serial.SendData((char*)bytes, p->getByteLength());

		lastBrake = current;
	}
}

void QuadInterface::setDesiredGear(Gear g) {
	if (!connected)
		return;	

	current = std::chrono::high_resolution_clock::now();
	seconds = current - lastGear;

	if (seconds.count() > 0.100) {
		Packet* p = new Packet();

		p->packetID = ID_SET_QUAD_GEAR;
		p->length = 1;
		p->data = new float[1];
		p->data[0] = (float)g;

		uint8_t* bytes = p->toBytes();

		serial.SendData((char*)bytes, p->getByteLength());

		delete bytes;
		delete p;


		lastGear = current;
		
	}
}

void QuadInterface::emergencyStop() {
	Packet* p = new Packet();
	p->packetID = ID_EMERGENCY_STOP;

	uint8_t* bytes = p->toBytes();
	serial.SendData((char*)bytes, p->getByteLength());

	delete bytes;
	delete p;
}

void QuadInterface::updateVelocityActuators() {
	double throttlePercentageRequired = abs(desiredVelocity) * 5;

	if (desiredVelocity == 0) {
		setDesiredThrottlePercentage(0);
		setDesiredGear(GEAR_NEUTRAL);
		setDesiredBrakePercentage(50);
	}
	else if (desiredVelocity > 0) {
		// if we are travelling in the wrong direction
		if (getVelocity() < 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrakePercentage(50);
			return;
		}
		// if we are travelling in the correct direction
		else {
			setDesiredGear(GEAR_FORWARD);
			setDesiredBrakePercentage(0);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (desiredVelocity < idleSpeed) {
			if (getVelocity()  < desiredVelocity) {
				setDesiredThrottlePercentage(0);
				setDesiredBrakePercentage(0);
			}
			else {
				setDesiredThrottlePercentage(0);
				setDesiredBrakePercentage(50);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large 
		// difference in actual speed and desired speed
		else {
			setDesiredThrottlePercentage(throttlePercentageRequired);
		}
	}
	else if (desiredVelocity < 0) {
		// if we are travelling in the wrong direction
		if (getVelocity()  > 0) {
			setDesiredThrottlePercentage(0);
			setDesiredGear(GEAR_NEUTRAL);
			setDesiredBrakePercentage(50);
			return;
		}
		// if we are travelling in the correct direction
		else {
			setDesiredGear(GEAR_REVERSE);
			setDesiredBrakePercentage(0);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (abs(desiredVelocity) < idleSpeed) {
			if (getVelocity()  < desiredVelocity) {
				setDesiredThrottlePercentage(0);
				setDesiredBrakePercentage(50);
			}
			else {
				setDesiredThrottlePercentage(0);
				setDesiredBrakePercentage(0);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large
		// difference in actual speed and desired speed
		else {
			//REMEMBER WE'RE IN REVERSE HERE
			setDesiredThrottlePercentage(throttlePercentageRequired);
		}
	}
}