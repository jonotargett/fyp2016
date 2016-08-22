#pragma once

/*
*
* CSerial code originally written by y Tom Archer and Rick Leinecker.
* CSerial - A C++ Class for Serial Communications
* Posted by Tom Archer and Rick Leinecker on August 7th, 1999
* http://www.codeguru.com/cpp/i-n/network/serialcommunications/article.php/c2503/CSerial--A-C-Class-for-Serial-Communications.htm
*
*/


#include <Windows.h>

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

class CSerial
{

public:
	CSerial();
	~CSerial();

	BOOL Open(int nPort = 2, int nBaud = 9600);
	BOOL Close(void);

	int ReadData(void *, int);
	int SendData(const char *, int);
	int ReadDataWaiting(void);

	BOOL IsOpened(void) { return(m_bOpened); }

protected:
	BOOL WriteCommByte(unsigned char);

	HANDLE m_hIDComDev;
	OVERLAPPED m_OverlappedRead, m_OverlappedWrite;
	BOOL m_bOpened;

};
