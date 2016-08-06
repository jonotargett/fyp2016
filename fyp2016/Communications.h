#pragma once

#include <vector>
#include <queue>
#include "SDL\SDL_Net.h"

#include "Log.h"
#include "Packet.h"

//#include "Thread.h"
#include <thread>
#include <chrono>

#define MAXLEN 1024
#define POLL 100		// milliseconds of inactivity before sending synchronous idle keep-alive
#define TIMEOUT 10000	// milliseconds before connection deemed inactive




class Communications
{
public:
	Communications();
	Communications(int);
	~Communications();

	bool isAlive();
	bool hasClient;

	bool initialise();
	void start();
	void close();

	bool acceptClient();
	bool communicationsLoop();
	bool send(Packet&);

private:
	const int socket;
	IPaddress ip;
	TCPsocket server;
	TCPsocket client;

	//Thread* updater;
	std::thread* updater;
	bool alive;
	std::queue<uint8_t> receivedBuffer;
	std::queue<Packet*> sendBuffer;

	std::chrono::time_point<std::chrono::high_resolution_clock> lastReceived;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastSent;
	std::chrono::time_point<std::chrono::high_resolution_clock> current;

	char* formatIP(Uint32);

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