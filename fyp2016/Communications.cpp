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
		Log::e << "SDLNet_ResolveHost: " << SDLNet_GetError() << endl;
		return false;
	}
	Log::i << "Localhost resolved... 127.0.0.1 (" << ip.host << ")" << endl;

	server = SDLNet_TCP_Open(&ip);
	if (!server) {
		Log::e << "SDLNet_TCP_Open: " << SDLNet_GetError() << endl;
		return false;
	}
	Log::i << "Server socket opened... " << socket << " (" << ip.port << ")" << endl;

	start();

	return true;
}

void Communications::start() {
	// start the updater thread
	delete updater;
	updater = NULL;
	alive = true;

	std::auto_ptr<Runnable> r(new CommsUpdaterRunnable(this));
	updater = new Thread(r);
	updater->start();
	Log::i << "Communiation sub-thread started." << std::endl;
}

void Communications::close() {
	SDLNet_TCP_Close(client);
	SDLNet_TCP_Close(server);
}

char* Communications::formatIP(Uint32 addr) {
	int b1, b2, b3, b4;

	b1 = (addr & 0x000000FF);
	b2 = (addr & 0x0000FF00) >> 8;
	b3 = (addr & 0x00FF0000) >> 16;
	b4 = (addr & 0xFF000000) >> 24;

	char* buf = new char[16];
	sprintf_s(buf, 16, "%i.%i.%i.%i", b1, b2, b3, b4);

	return buf;
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
		IPaddress* clientAddr = SDLNet_TCP_GetPeerAddress(client);
		Log::i << "Client " << formatIP(clientAddr->host) << " connected to server socket." << std::endl;
	}
	return hasClient;
}

bool Communications::communicationsLoop() {

	char msg = 'a';// 0x16;	// synchronous idle character, a keep-alive indicating no data
	int result = -1;

	SDLNet_SocketSet set;
	set = SDLNet_AllocSocketSet(1);
	SDLNet_TCP_AddSocket(set, client);

	SDLNet_TCP_Send(client, &msg, 1);
	result = SDLNet_CheckSockets(set, 5000);

	if (result <= 0) {
		Log::e << endl << "Communications Error: client has reached unresponsive timeout" << endl;
		hasClient = false;
		return false;
	}

	result = SDLNet_TCP_Recv(client, &msg, 1);

	if (result <= 0) {
		Log::e << std::endl <<  "Communications Error: client has disconnected." << std::endl;
		hasClient = false;
		return false;
	}

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