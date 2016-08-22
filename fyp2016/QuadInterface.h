#pragma once

/*

NOTE FOR HARRY: IF YOURE MERGING YOUR QUADBIKE CLASS;
THIS IS NOT THE PLACE TO DO IT. YOUR VIRTUAL QUADBIKE
BELONGS EITHER IN THE SAME FILE YOU MADE IT, BUT
REARRANGED TO IMPLEMENT THE HARDWARE INTERFACE SPECS,
OR MERGE WITH DUMMY HARDWARE.

THIS IS WHERE THE ACTUAL HARDWARE INTERFACE STUFF BELONGS.

*/

#define COM_PORT 4			// if COM port is known
#define MAX_COM_SEARCH 10		// search between COM1 and COMx
#define SEARCH_INTERVAL 1000	// time to wait between checking successive COM ports
#define BAUD_RATE 115200
#define COM_TIMEOUT 50			// maximum wait time before severing COM 


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

