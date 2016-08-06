#include "Communications.h"



Communications::Communications() : socket(2099)
{
	SDLNet_Init();
	hasClient = false;
	alive = true;
	collectingPacket = false;
}

Communications::Communications(int s) : socket(s) {
	SDLNet_Init();
	hasClient = false;
	alive = true;
	collectingPacket = false;
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

	//std::thread* thread = new std::thread(run, this);

	updater = new std::thread(&Communications::communicationsLoop, this);

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
	sprintf(buf, "%i.%i.%i.%i", b1, b2, b3, b4);

	return buf;
}

bool Communications::acceptClient() {
	if (hasClient)
		return hasClient;

	client = NULL;
	client = SDLNet_TCP_Accept(server);

	// prevent this from going overboard and pinging for clients
	// unecessarily. hold ye horses o' computah
	//SDL_Delay(50);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	if (client) {
		hasClient = true;
		IPaddress* clientAddr = SDLNet_TCP_GetPeerAddress(client);
		Log::i << "Client " << formatIP(clientAddr->host) << " connected to server socket." << std::endl;
	}
	return hasClient;
}


bool Communications::isConnected() {
	return hasClient;
}

bool Communications::send(Packet* p) {
	sendBuffer.push(p);

	return true;
}
 

bool Communications::communicationsLoop() {

	SDLNet_SocketSet set;
	set = SDLNet_AllocSocketSet(1);

	while (isAlive()) {

		if (!hasClient) {
			acceptClient();
		}
		else {
			current = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> seconds;

			uint8_t msg = ID_IDLE;	// synchronous idle character, a keep-alive indicating no data
			int result = -1;

			
			SDLNet_TCP_AddSocket(set, client);

			

			// HANDLE OUTGOING COMMUNICATIONS ------------------------------------------

			if (sendBuffer.size() > 0) {
//there are packets waiting to be sent. 

int packets = sendBuffer.size();

// handle those
while (sendBuffer.size() > 0) {
	Log::d << "sending outgoing packet "
		<< (packets - sendBuffer.size() + 1)
		<< " of " << (packets) << "..." << endl;

	Packet* p = sendBuffer.front();

	uint8_t* bytes = p->toBytes();
	uint8_t len = p->getByteLength();
	result = SDLNet_TCP_Send(client, bytes, len);

	if (result < len) {
		Log::e << "Communications Error: interrupted incomplete transmission" << endl;
	}


	delete bytes;
	sendBuffer.pop();
}


			}
			else {
				seconds = current - lastSent;

				if (seconds.count() * 1000 > POLL) {
					// havent sent a packet in a while, quad is going to think
					// that we have lost comms. so, send a keep-alive packet
					// just so it knows we're still here

					SDLNet_TCP_Send(client, &msg, 1);
					lastSent = std::chrono::high_resolution_clock::now();
				}
			}



			// CHECK FOR INCOMING COMMUNICATIONS ---------------------------------------

			// thread blocking check
			//result = SDLNet_CheckSockets(set, TIMEOUT);
			result = SDLNet_CheckSockets(set, 0);

			if (result <= 0) {
				seconds = current - lastReceived;

				if (seconds.count() * 1000 > TIMEOUT) {
					Log::e << endl << "Communications Error: client has reached unresponsive timeout" << endl;
					hasClient = false;
				}
			}
			else {
				lastReceived = std::chrono::high_resolution_clock::now();

				result = SDLNet_TCP_Recv(client, &msg, 1);

				if (result <= 0) {
					Log::e << std::endl << "Communications Error: client has disconnected." << std::endl;
					hasClient = false;
				}
				else {
					if (msg == ID_SOH) {
						collectingPacket = true;
					}
					else if (msg == ID_ETB) {
						collectingPacket = false;
						processPacket();
					}
					else if (collectingPacket) {
						receivedBuffer.push(msg);
					}
				}
			}



			SDLNet_TCP_DelSocket(set, client);
		}
	}

	return true;
}



bool Communications::processPacket() {
	Packet* p = new Packet();

	p->packetID = (ID)receivedBuffer.front();
	receivedBuffer.pop();
	p->length = receivedBuffer.front();
	receivedBuffer.pop();

	p->data = new float[p->length];

	if (receivedBuffer.size() == (p->length*4)) {
		Log::e << "completed packet" << endl;
	}
	else {
		Log::e << "invalid packet length" << endl;
		delete p;
		return false;
	}

	for (int i = 0; i < p->length; i++) {
		float result;
		uint8_t b0 = receivedBuffer.front();
		receivedBuffer.pop();
		uint8_t b1 = receivedBuffer.front();
		receivedBuffer.pop();
		uint8_t b2 = receivedBuffer.front();
		receivedBuffer.pop();
		uint8_t b3 = receivedBuffer.front();
		receivedBuffer.pop();

		uint8_t byte_array[] = { b0, b1, b2, b3 };

		std::copy(reinterpret_cast<const uint8_t*>(&byte_array[0]),
			reinterpret_cast<const uint8_t*>(&byte_array[4]),
			reinterpret_cast<uint8_t*>(&result));

		p->data[i] = result;
	}

	Log::i << "PACKET RECEIVED: " << (int)p->packetID << " / " << (int)p->length << endl;
	Log::d << p->data[0] << "/" << p->data[1] << "/" << p->data[2] << "/" << p->data[3] << endl;

	return true;
}

