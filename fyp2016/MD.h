#pragma once
#pragma pack(1)


#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <winsock.h>	// this doesnt need to be included for any reason than
// to get rid of those damn macro-redefinition errors
// that SDL_Net generates if the include order isnt correct

#include "SDL\SDL_Net.h"
#include "Log.h"



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
		memcpy(this, payload, sizeof(StreamData));
	}

};


class MD
{
public:
	MD();
	~MD();


	bool initialise();
	StreamData* getFrame(unsigned int = 0);

private:
	const int socket;
	IPaddress ip;
	TCPsocket server;

	bool alive;

	std::thread* updater;

	void start();
	bool acquisitionLoop();

	std::vector<StreamData*> frames;
};


