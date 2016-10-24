#pragma once
//#pragma pack(1)		IS CAUSING A MASSIVE HEAP CORRUPTION


#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <winsock.h>	// this doesnt need to be included for any reason than
// to get rid of those damn macro-redefinition errors
// that SDL_Net generates if the include order isnt correct

#include "SDL\SDL_Net.h"
#include "SDL\SDL.h"
#include "Log.h"
#include "SimpleTexture.h"



#define MAX_FRAMES_STORED 100000


struct StreamData
{
	uint8_t packet_type;
	uint32_t size_bytes;
	uint8_t StreamId;
	uint8_t ItemCount;
	int32_t TimeStamp;
	int32_t Data[3 * 4 * 2];
	//int32_t Data[1 + 3 * 4 * 2];  // for AMDS (RAW frame)

	void deserialize(uint8_t* payload) {
		//memcpy(this, payload, sizeof(StreamData));

		//Log::i << "Starting a packet -------------------------------" << endl;
		
		unsigned int offset = 0;

		packet_type = payload[offset++];
		//Log::i << "Offset: " << offset << endl;
		size_bytes = getInt(payload, offset);
		offset += 4;
		//Log::i << "Offset: " << offset << endl;
		StreamId = payload[offset++];
		//Log::i << "Offset: " << offset << endl;
		ItemCount = payload[offset++];
		//Log::i << "Offset: " << offset << endl;
		TimeStamp = getInt(payload, offset);
		offset += 4;
		//Log::i << "Offset: " << offset << endl;

		for (int i = 0; i < 3 * 4 * 2; i++) {
			Data[i] = getInt(payload, offset);
			offset += 4;
			//Log::i << "Offset: " << offset << endl;
		}
	}

	int32_t getInt(uint8_t* payload, unsigned int offset) {
		union u_tag {
			uint8_t b[4];
			uint32_t val;
		} u;

		u.b[0] = payload[offset++];
		u.b[1] = payload[offset++];
		u.b[2] = payload[offset++];
		u.b[3] = payload[offset++];

		return u.val;
	}

};


struct Signal {
	uint32_t p;
	uint32_t q;
};

struct Channel {
	Signal freq[4];
};

struct Frame {
	uint32_t timestamp;
	Channel channel[3];
};

class MD
{
public:
	MD(SDL_Renderer*);
	~MD();


	bool initialise();
	Frame* getFrame(unsigned int = 0);

	SDL_Texture* retrieveMDImage();
	void updateMDImage();

private:
	const int socket;
	IPaddress ip;
	TCPsocket server;

	bool alive;
	std::vector<Frame*> frames;
	SDL_Texture* mdTexture;
	SDL_Renderer* renderer;

	std::thread* updater;

	void start();
	bool acquisitionLoop();
};


