#pragma once

#include <vector>
#include "SDL\SDL_Net.h"

#include "Log.h"

//#include "Thread.h"
#include <thread>
#include <chrono>

#define MAXLEN 1024
#define TIMEOUT 1000	// milliseconds before connection deemed inactive

struct Packet {
	char packetID;
	int length;
	float* data;

	Packet() {

	}
};


class Communications
{
private:
	const int socket;
	IPaddress ip;
	TCPsocket server;
	TCPsocket client;

	//Thread* updater;
	std::thread* updater;
	bool alive;
	std::vector<char> receivedBuffer;

	char* formatIP(Uint32);
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