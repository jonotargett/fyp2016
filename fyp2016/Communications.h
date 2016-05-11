#pragma once

#include <iostream>
#include <vector>
#include "SDL\SDL_Net.h"

#include "Thread.h"

#define MAXLEN 1024

class Communications
{
private:
	const int socket;
	IPaddress ip;
	TCPsocket server;
	TCPsocket client;

	Thread* updater;
	bool alive;
	std::vector<char> receivedBuffer;
public:
	Communications();
	Communications(int);
	~Communications();
	bool isAlive();
	bool hasClient;

	bool initialise();
	void close();

	bool acceptClient();

	bool communicationsLoop();
};


class CommsUpdaterRunnable : public Runnable {
private:
	Communications* comms;
public:
	CommsUpdaterRunnable(Communications* c) : comms(c) {	}
	virtual void* run();
};