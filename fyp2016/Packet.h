#pragma once
#include <cstdlib>
#include <cstdint>

// PACKET IDS

enum ID : uint8_t {
	ID_NULL = 0x00,
	ID_EOT = 0x04,
	ID_IDLE = 0x16,
	ID_ETB = 0x17,
	ID_CANCEL = 0x18,

	ID_DEBUG = 0x40
};


struct Packet {
	ID packetID;
	uint8_t length;
	float* data;

	Packet() {
		packetID = ID_NULL;
		length = 0;
		data = NULL;
	}

	uint8_t getByteLength() {
		return (2 + length * 4);
	}

	uint8_t* toBytes() {
		int byte_length = getByteLength();

		uint8_t* bytes = new uint8_t[byte_length];

		bytes[0] = (uint8_t)packetID;
		bytes[1] = (uint8_t)length;
		uint8_t offset = 1;

		for (uint8_t i = 0; i < length; i++) {

			float f = data[i];
			uint8_t const* p = reinterpret_cast<uint8_t const*>(&f);

			for (uint8_t j = 0; j < 4; ++j) {
				bytes[++offset] = p[j];
			}
		}

		bytes[++offset] = 0x17;

		return bytes;
	}
};