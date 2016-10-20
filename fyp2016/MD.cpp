#include "MD.h"



MD::MD() : socket(6340) {
	alive = true;
}


MD::~MD() {
}


bool MD::initialise() {
	/*
	Opens a port on the current device, as a server.
	A client will then need to connect to this device.
	*/

	if (SDLNet_ResolveHost(&ip, "localhost", socket) == -1) {
		Log::e << "SDLNet_ResolveHost: " << SDLNet_GetError() << endl;
		return false;
	}

	server = SDLNet_TCP_Open(&ip);
	if (!server) {
		Log::e << "SDLNet_TCP_Open: " << SDLNet_GetError() << endl;
		return false;
	}
	Log::i << "Connected to server... " << socket << " (" << ip.port << ")" << endl;

	// starts our subthread.
	start();


	return true;
}



void MD::start() {

	//std::thread* thread = new std::thread(run, this);

	updater = new std::thread(&MD::acquisitionLoop, this);

	Log::d << "Communiation sub-thread started." << std::endl;
}


bool MD::acquisitionLoop() {

	while (alive) {

		// arriving packet structure is:

		//	uint8_t packet_type;				// irrelevant, always the same
		//	uint32_t size_bytes;				// irrelevant, always the same
		//	uint8_t StreamId;					// dunno
		//	uint8_t ItemCount;					// irrelevant
		//	int32_t TimeStamp;
		//	int32_t Data[3 * 4 * 2];			// for AMDS (RAW frame) : ONLY IMPORTANT PART

		// packet size is therefore:
		unsigned int packetSize = 1 + 4 + 1 + 1 + 4 + 4*(3 * 4 * 2);
		uint8_t* packetBytes = new uint8_t[packetSize];


		// this is a thread blocking call. which is fine - this thread doesnt do anything else.
		int result = SDLNet_TCP_Recv(server, packetBytes, packetSize);

		if (result < 0) {
			Log::e << "MD communications interrupted" << endl;
			alive = false;
			return false;
		}

		StreamData* packet = new StreamData();
		packet->deserialize(packetBytes);
		delete packetBytes;

		frames.push_back(packet);

		//stop us from chewing memory - this should limit to about 15 MiB, plenty of headroom
		if (frames.size() < MAX_FRAMES_STORED) {
			frames.erase(frames.begin());
		}
	}

	return true;
}



StreamData* MD::getFrame(unsigned int index = 0) {
	if (index >= frames.size()) {
		Log::e << "Requested frame is outside of bounds of array" << endl;
		return NULL;
	}

	return frames.at(frames.size() - 1 - index);
}