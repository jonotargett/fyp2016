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

	//if (SDLNet_ResolveHost(&ip, "localhost", socket) == -1) {
	if (SDLNet_ResolveHost(&ip, "192.168.43.139", socket) == -1) {
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


		StreamData* packet = new StreamData();
		packet->deserialize(packetBytes);
		delete packetBytes;

		//std::this_thread::sleep_for(std::chrono::microseconds(100));
		/*
		Log::i << "Packet Type:\t 0x" << std::hex << (int)packet->packet_type << std::dec << endl;
		Log::i << "Packet size:\t 0x" << std::hex << (int)packet->size_bytes << std::dec << endl;
		Log::i << "Packet stream:\t 0x" << std::hex << (int)packet->StreamId << std::dec << endl;
		Log::i << "Packet count:\t 0x" << std::hex << (int)packet->ItemCount << std::dec << endl;
		Log::i << "Packet timest:\t 0x" << std::hex << (int)packet->TimeStamp << std::dec << endl;
		Log::i << "Packet data 1:\t 0x" << std::hex << (int)packet->Data[0] << std::dec << endl;
		Log::i << "Packet data 2:\t 0x" << std::hex << (int)packet->Data[1] << std::dec << endl;
		*/

		Frame* newFrame = new Frame();
		unsigned int offset = 0;

		newFrame->timestamp = packet->TimeStamp;

		newFrame->channel[0].freq[0].p = (int)packet->Data[offset++];
		newFrame->channel[0].freq[0].q = (int)packet->Data[offset++];
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

		current = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> seconds;
		seconds = current - last;

		if (seconds.count() > (1.00)) {
			last = current;

			//Log::i << "newFrame: " << newFrame->channel[0].freq[0].p << " / " << newFrame->channel[0].freq[0].q << endl;
		}

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


float MD::getMagnitude() {
	return sqrt((pVal * pVal) + (qVal * qVal));
}


void MD::updateMDImage() {
	SDL_DestroyTexture(mdTexture);
	mdTexture = NULL;

	int m = 512;

	SDL_Surface* image = SDL_CreateRGBSurface(0, m, m, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	uint32_t *pixels = (uint32_t*)image->pixels;

	// initialise the surface to blue
	//for (int i = 0; i < image->w*image->h; i++) {
	//	pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
	//}

	// initialise the surface to white?
	memset(pixels, 0xFF, m*m*sizeof(uint32_t));

	for (int i = 0; i < m; i++) {
		pixels[i*m + m/2] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
		pixels[(m/2)*m + i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
	}


	int num = min(150, frames.size());

	for (int i = 0; i < num; i++) {
		Frame* frame = getFrame(i);

		for (int j = 0; j < 3; j++) {
			float x = frame->channel[j].freq[0].p;// *(float)(m / 10000.0f);
			float y = frame->channel[j].freq[0].q;// *(float)(m / 10000.0f);

			if (sqrt((x*x) + (y*y)) > getMagnitude()){
				pVal = x;
				pVal = y;
			}

			x /= 10000000;
			y /= 10000000;

			x = (x + 1.0) / 2.0;
			y = (y + 1.0) / 2.0;

			x *= m;
			y *= m;

			x = min(max(0, x), m - 1);
			y = min(max(0, y), m - 1);

			switch (j) {
			case 0:
				pixels[(int)x*m + (int)y] = SDL_MapRGB(image->format, 0xFF, 0x00, 0x00);
				break;
			case 1:
				pixels[(int)x*m + (int)y] = SDL_MapRGB(image->format, 0x00, 0xFF, 0x00);
				break;
			case 2:
				pixels[(int)x*m + (int)y] = SDL_MapRGB(image->format, 0x00, 0x00, 0xFF);
				break;
			default:
				break;
			}
		}
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