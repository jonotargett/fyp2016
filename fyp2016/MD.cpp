#include "MD.h"



MD::MD(SDL_Renderer* r) : socket(6340), renderer(r) {
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
	//if (SDLNet_ResolveHost(&ip, "192.168.1.122", socket) == -1) {
		Log::e << "SDLNet_ResolveHost: " << SDLNet_GetError() << endl;
		return false;
	}

	server = SDLNet_TCP_Open(&ip);
	if (!server) {
		Log::e << "SDLNet_TCP_Open: " << SDLNet_GetError() << endl;
		return false;
	}
	Log::i << "Connected to MD server... " << socket << " (" << ip.port << ")" << endl;

	uint8_t initData[5] = { 1, 0, 0, 0, 0 };

	SDLNet_TCP_Send(server, initData, 5);
	Log::i << "Sent initialisation packet to server..." << endl;

	// starts our subthread.
	start();


	return true;
}



void MD::start() {

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
			delete packetBytes;
			return false;
		}
		if (result < packetSize) {
			Log::e << "MD communications invalid packet" << endl;
			delete packetBytes;
			continue;
		}

		Log::i << "------------------------------------------------------------" << endl <<
			"recieved  " << result << " / " << packetSize << "  bytes of data." << endl;

		StreamData* packet = new StreamData();
		packet->deserialize(packetBytes);
		delete packetBytes;

		//std::this_thread::sleep_for(std::chrono::microseconds(100));

		//Log::i << "Packet Type:\t 0x" << std::hex << (int)packet->packet_type << std::dec << endl;
		//Log::i << "Packet size:\t 0x" << std::hex << (int)packet->size_bytes << std::dec << endl;
		//Log::i << "Packet stream:\t 0x" << std::hex << (int)packet->StreamId << std::dec << endl;
		//Log::i << "Packet count:\t 0x" << std::hex << (int)packet->ItemCount << std::dec << endl;
		//Log::i << "Packet timest:\t 0x" << std::hex << (int)packet->TimeStamp << std::dec << endl;
		//Log::i << "Packet data:\t 0x" << std::hex << (int)packet->Data[0] << std::dec << endl;


		Frame* newFrame = new Frame();
		unsigned int offset = 0;

		newFrame->timestamp = packet->TimeStamp;

		newFrame->channel[0].freq[0].p = packet->Data[offset++];
		newFrame->channel[0].freq[0].q = packet->Data[offset++];
		newFrame->channel[0].freq[1].p = packet->Data[offset++];
		newFrame->channel[0].freq[1].q = packet->Data[offset++];
		newFrame->channel[0].freq[2].p = packet->Data[offset++];
		newFrame->channel[0].freq[2].q = packet->Data[offset++];
		newFrame->channel[0].freq[3].p = packet->Data[offset++];
		newFrame->channel[0].freq[3].q = packet->Data[offset++];

		newFrame->channel[1].freq[0].p = packet->Data[offset++];
		newFrame->channel[1].freq[0].q = packet->Data[offset++];
		newFrame->channel[1].freq[1].p = packet->Data[offset++];
		newFrame->channel[1].freq[1].q = packet->Data[offset++];
		newFrame->channel[1].freq[2].p = packet->Data[offset++];
		newFrame->channel[1].freq[2].q = packet->Data[offset++];
		newFrame->channel[1].freq[3].p = packet->Data[offset++];
		newFrame->channel[1].freq[3].q = packet->Data[offset++];

		newFrame->channel[2].freq[0].p = packet->Data[offset++];
		newFrame->channel[2].freq[0].q = packet->Data[offset++];
		newFrame->channel[2].freq[1].p = packet->Data[offset++];
		newFrame->channel[2].freq[1].q = packet->Data[offset++];
		newFrame->channel[2].freq[2].p = packet->Data[offset++];
		newFrame->channel[2].freq[2].q = packet->Data[offset++];
		newFrame->channel[2].freq[3].p = packet->Data[offset++];
		newFrame->channel[2].freq[3].q = packet->Data[offset++];


		frames.push_back(newFrame);
		delete packet;

		//stop us from chewing memory - this should limit to about 15 MiB, plenty of headroom
		if (frames.size() > MAX_FRAMES_STORED) {
			frames.erase(frames.begin());
		}
	}

	return true;
}



Frame* MD::getFrame(unsigned int index) {
	if (index >= frames.size()) {
		Log::e << "Requested frame is outside of bounds of array" << endl;
		return NULL;
	}

	return frames.at(frames.size() - 1 - index);
}





void MD::updateMDImage() {
	SDL_DestroyTexture(mdTexture);
	mdTexture = NULL;


	SDL_Surface* image = SDL_CreateRGBSurface(0, 256, 256, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	uint32_t *pixels = (uint32_t*)image->pixels;

	// initialise the surface to blue
	//for (int i = 0; i < image->w*image->h; i++) {
	//	pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
	//}

	// initialise the surface to white?
	memset(pixels, 0xFF, image->w*image->h*sizeof(uint32_t));

	for (int i = 0; i < image->h; i++) {
		pixels[i*image->w + image->w/2] = SDL_MapRGB(image->format, 0x00, 0x60, 0xFF);
		pixels[(image->h/2)*image->w + i] = SDL_MapRGB(image->format, 0x00, 0x60, 0xFF);
	}


	int num = frames.size();

	for (int i = 0; i < num; i++) {
		Frame* frame = getFrame(i);

		uint16_t x = frame->channel[0].freq[0].p / (4294967295 / image->h);
		uint16_t y = frame->channel[0].freq[0].q / (4294967295 / image->w);

		pixels[x*image->w + y] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
	}


	mdTexture = SDL_CreateTextureFromSurface(renderer, image);

	if (mdTexture == NULL) {
		Log::e << "Could not create texture from scan image. " << SDL_GetError() << endl;
	}

	SDL_FreeSurface(image);

	return;
}


SDL_Texture* MD::retrieveMDImage() {
	return mdTexture;
}