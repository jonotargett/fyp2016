#include "Communications.h"



Communications::Communications() : socket(2099)
{
	SDLNet_Init();
	hasClient = false;
	alive = true;
}

Communications::Communications(int s) : socket(s) {
	SDLNet_Init();
	hasClient = false;
	alive = true;
}


Communications::~Communications()
{
	SDLNet_Quit();
	alive = false;
}

bool Communications::isAlive() {
	return alive;
}

bool Communications::initialise() {
	/*
	Opens a port on the current device, as a server.
	A client will then need to connect to this device.
	*/

	if (SDLNet_ResolveHost(&ip, NULL, socket) == -1) {
		Log::e << "SDLNet_ResolveHost: " << SDLNet_GetError() << std::endl;
		return false;
	}
	Log::i << "Localhost resolved... " << ip.host << std::endl;

	server = SDLNet_TCP_Open(&ip);
	if (!server) {
		Log::e << "SDLNet_TCP_Open: " << SDLNet_GetError() << std::endl;
		return false;
	}
	Log::i << "Server socket opened... " << ip.port << std::endl;

	// start the updater thread
	delete updater;
	updater = NULL;
	alive = true;

	std::auto_ptr<Runnable> r(new CommsUpdaterRunnable(this));
	updater = new Thread(r);
	updater->start();
	Log::i << "Communiation sub-thread started." << std::endl;
	
	return true;
}

void Communications::close() {
	SDLNet_TCP_Close(client);
	SDLNet_TCP_Close(server);
}

bool Communications::acceptClient() {
	if (hasClient)
		return hasClient;

	client = SDLNet_TCP_Accept(server);

	// prevent this from going overboard and pinging for clients
	// unecessarily. hold ye horses o' computah
	SDL_Delay(50);

	if (client) {
		hasClient = true;
		Log::i << "Client connected to server socket." << std::endl;
	}
	return hasClient;
}

bool Communications::communicationsLoop() {

	char msg;
	int result = -1;

	result = SDLNet_TCP_Recv(client, &msg, 1);

	if (result <= 0) {
		Log::e << std::endl <<  "Communications Error: client is disconnected." << std::endl;
		hasClient = false;
		return false;
	}

	// dump whatever we receive straight to cout.
	std::cout << msg;
	receivedBuffer.push_back(msg);

	return true;
}


void* CommsUpdaterRunnable::run() {

	bool success = true;

	while (comms->isAlive()) {

		if (!comms->hasClient) {
			comms->acceptClient();
		}
		else {
			success = comms->communicationsLoop();
		}
	}

	return (void*)true;
}