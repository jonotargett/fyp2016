#pragma once

/*

NOTE FOR HARRY: IF YOURE MERGING YOUR QUADBIKE CLASS;
THIS IS NOT THE PLACE TO DO IT. YOUR VIRTUAL QUADBIKE
BELONGS EITHER IN THE SAME FILE YOU MADE IT, BUT
REARRANGED TO IMPLEMENT THE HARDWARE INTERFACE SPECS,
OR MERGE WITH DUMMY HARDWARE.

THIS IS WHERE THE ACTUAL HARDWARE INTERFACE STUFF BELONGS.

*/

//#define COM_PORT 4
#define MAX_COM_SEARCH 10
#define BAUD_RATE 115200
#define COM_TIMEOUT 1000

#include <queue>

#include "HardwareInterface.h"
#include "Serial.h"
#include "Packet.h"


class QuadInterface :
	public HardwareInterface
{
public:
	QuadInterface();
	~QuadInterface();
	bool initialise();
	bool updateLoop();

private:
	Packet* processPacket();
	bool establishCOM(int);

	CSerial serial;
	int comPort;
	std::queue<uint8_t> receivedBuffer;
	
};

