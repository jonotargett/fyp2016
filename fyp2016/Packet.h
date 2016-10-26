#pragma once
#include <cstdlib>
#include <cstdint>

#include "Log.h"

// PACKET IDS

enum ID : uint8_t {
	ID_NULL = 0x00,
	ID_SOH = 0x01,			// start of transmission block
	ID_EOT = 0x04,			// end of transmission
	ID_IDLE = 0x16,			// synchronous idle
	ID_ETB = 0x17,			// end of transmission block
	ID_CANCEL = 0x18,		// transmission cancelled (throw away whats received, 
							// urgent shit coming like an e-stop command)

	ID_READY = 0x05,
	ID_DEBUG = 0x40,
	ID_SHOW_FD = 0x41,
	ID_SHOW_VP = 0x42,

	ID_QUAD_POSITION = 0x50,			// single float
	ID_QUAD_HEADING = 0x51,				// single float
	ID_QUAD_SPEED = 0x52,				// single float
	ID_REQ_QUAD_POSITION = 0x53,
	ID_REQ_QUAD_HEADING = 0x54,
	ID_REQ_QUAD_SPEED = 0x55,
	ID_QUAD_RESETPOS = 0x56,

	ID_CLEAR_NAV_POINTS = 0x60,
	ID_NAV_POINTS = 0x61,
	ID_NAV_PATH = 0x62,					// doubles array [lat, lon]
	ID_NAV_ZONE = 0x63,					// doubles array [lat, lon]
	ID_NAV_BASELOC = 0x64,
	ID_NAV_GENERATE = 0x65,
	ID_AUTO_NAV_ON = 0x66,
	ID_AUTO_NAV_OFF = 0x67,

	ID_SET_QUAD_THROTTLE = 0x70,
	ID_SET_QUAD_STEERING = 0x71,
	ID_SET_QUAD_BRAKE = 0x72,
	ID_SET_QUAD_GEAR = 0x73,
	ID_REQ_QUAD_THROTTLE = 0x74,
	ID_REQ_QUAD_STEERING = 0x75,
	ID_REQ_QUAD_BRAKE = 0x76,
	ID_REQ_QUAD_GEAR = 0x77,
	ID_QUAD_THROTTLE = 0x78,			// single float
	ID_QUAD_STEERING = 0x79,			// single float
	ID_QUAD_BRAKE = 0x7A,				// single float
	ID_QUAD_GEAR = 0x7B,				// single float, -1 rev, 0 neutral, 1 forward
	ID_QUAD_GPS = 0x7C,					// two floats, [lat, lon]
	ID_QUAD_IMU = 0x7D,					// single float

	ID_STOP_ENGINE = 0x80,
	ID_HANDBRAKE_ON = 0x81,
	ID_HANDBRAKE_OFF = 0x82,
	ID_MANUALCONTROL_ON = 0x83,
	ID_MANUALJOYSTICK = 0x84,
	ID_BRAKE = 0x85,
	ID_JOYSTICK_HELD = 0x86,
	ID_JOYSTICK_RELEASED = 0x87,

	ID_EMERGENCY_STOP = 0xFF

};


/*
PACKET STRUCTURE:

PACKET ID: this will be something from the enum above.
			the value of this ID will indicate to the receiver what the
			packet data is. This could be a command, or an indicator
			of what the data type is and where it should go to. 

PACKET LENGTH: indicates the length (IN FLOATS) of the float array
				passed in this packet. in the range 0 - 255. for 
				packets carrying no data, this will be zero. YOU MUST
				SET THIS LENGTH YOURSELF. THE PACKET STRUCT WONT 
				CALCULATE IT FOR YOU

PACKET DATA:	an array of floats, length as specified. 

*/

struct Packet {
	ID packetID;
	uint16_t length;
	float* data;
	uint8_t* bytes;

	Packet() {
		packetID = ID_NULL;
		length = 0;
		data = NULL;
		bytes = NULL;
	}
	~Packet() {
		if(data != NULL) delete data;
		data = NULL;
	}

	uint16_t getByteLength() {
		return (4 + length * 4);
	}

	uint8_t* toBytes() {
		int byte_length = getByteLength();
		if (byte_length > 254) {
			byte_length = 254;
		}

		bytes = new uint8_t[byte_length];

		bytes[0] = ID_SOH;
		bytes[1] = (uint8_t)packetID;
		bytes[2] = (uint8_t)length;
		uint8_t offset = 2;

		for (uint8_t i = 0; i < length; i++) {

			float f = data[i];
			uint8_t const* p = reinterpret_cast<uint8_t const*>(&f);

			for (uint8_t j = 0; j < 4; ++j) {
				if (offset < 254) {
					bytes[++offset] = p[j];
				}
			}
		}

		bytes[++offset] = ID_ETB;

		return bytes;
	}

	void fromBytes(uint8_t* b, uint16_t bl) {

		if (bl < 2)
			return;
		
		if(bytes != NULL)
			delete bytes;
		
		bytes = b;

		int offset = 0;
		packetID = (ID)bytes[offset];
		offset++;
		length = bytes[offset];
		offset++;

		if (bl != length * 4 + 2) return;
		if (bl > 255) {
			Log::e << "Packet is too long!" << endl;
			return;
		}
		if (data != NULL) {
			delete data;
		}
		data = new float[length];

		union u_tag {
			uint8_t b[4];
			float fval;
		} u;

		for (int i = 0; i < length; i++) {
			u.b[0] = bytes[offset++];
			u.b[1] = bytes[offset++];
			u.b[2] = bytes[offset++];
			u.b[3] = bytes[offset++];

			data[i] = u.fval;
		}

		return;
	}
};