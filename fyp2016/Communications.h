#pragma once

#include <vector>
#include <queue>
#include "SDL\SDL_Net.h"

#include "Log.h"
#include "Packet.h"
#include "CommsListener.h"

//#include "Thread.h"
#include <thread>
#include <chrono>

#define MAXLEN 1024
#define POLL 1000		// milliseconds of inactivity before sending synchronous idle keep-alive
#define TIMEOUT 3000	// milliseconds before connection deemed inactive




class Communications 
{
public:
	Communications();
	Communications(int);
	~Communications();

	bool initialise();

	bool acceptClient();
	bool communicationsLoop();
	bool send(Packet*);

	bool isConnected();
	void setListener(CommsListener*);

private:
	const int socket;
	IPaddress ip;
	TCPsocket server;
	TCPsocket client;

	bool hasClient;
	bool collectingPacket;

	//Thread* updater;
	std::thread* updater;
	bool alive;
	std::queue<uint8_t> receivedBuffer;
	std::queue<Packet*> sendBuffer;
	CommsListener* listener;

	std::chrono::time_point<std::chrono::high_resolution_clock> lastReceived;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastSent;
	std::chrono::time_point<std::chrono::high_resolution_clock> current;

	char* formatIP(Uint32);
	bool processPacket();
	bool isAlive();
	void start();
	void close();
};



/*
class CommsUpdaterRunnable : public Runnable {
private:
	Communications* comms;
public:
	CommsUpdaterRunnable(Communications* c) : comms(c) {	}
	virtual void* run();
};
*/