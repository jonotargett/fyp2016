#pragma once
/* This include file defines the DLL interface for GPR hardware.
There are two functions - one to set up the hardware and
one to get the data from the hardware. */
struct id_struct {

	int antenna_id;	// antenna id byte
	int low_battery; // 1 => low battery 
	char pga_id[30]; // fpga version and date id string
	char pgm_id[30]; // Firmware version and date id string
};

#define	DEVICE_OFFLINE			0x00000001
#define	RADAR_OFF				0x00000002
#define	INVALID_BUFFER_SIZE		0x00000004
#define	BAD_USB_SEQUENCE_NUMBER	0x00000008
#define	MICRO_BUFFER_OVERFLOW	0x00000010
#define	NOT_ENOUGH_DATA			0x00000020
#define	DATA_LOST				0x00000040
#define SAMPLES_IN_BUFFER		0x0000FF00
#define SAMPLES_RETURNED		0xFFFF0000

extern "C" __declspec(dllimport)
int read_data(unsigned int *buffer, int howmany, struct id_struct *gpr_id);

/* This function

int read_data(unsigned int *buffer,int howmany, struct id_struct *);

gets data from the radar

inputs

1. buffer  - a pointer to a buffer of ints;buffer size 1 to 8192
2. howmany - the buffer size (range -1..8192)

Note: -  -1 synchronises the data stream
0 just returns the status
1 - 8192 for regular usage

3. gpr_id  - structure for returning id


returns
1. the status code as the function return value.
2. the buffer containing data.
3. gpr_id  - structure with antenn and Firmware id

(the function returns with either no data or with the buffer completely
filled with data.)

The status code and data format is explained below.


status code bits (d31,d30....d0)

a return value is 0 indicates "all ok" and the buffer contains the
requested number of samples. a '1' in any bit position indicates that
some unusual condition has occurred.

d0		-	device offline
d1		-	radar is off
d8		-	invalid buffer size
d9		-	the dll buffer got full
(this is indicative only. no loss of data has yet occurred)
d10	-	bad sequence number on the USB
(some loss/repetition in data will be the result)
d11	-	Micro buffer overflow (loss of some data)
d24-d16 - Samples in Micro Buffer in units of 64.
range - 1 to 0xff;
a value of n => Micro Buffer has between 64*n and 64*(n+1) samples
d30 -	Synch operation succeeded
d31	-	not enough data in dll buffer; no data is returned

data word bits (d31,d30... d0)

d15-d0	- 16 bit A/D converter value; two's compliment
d23-d22	-	Antenna code(11 - differential ;10/01 - antenna1 / antenna2)
d21-d20	- Position sensor data
d19-d16	- switch data ( the four switches on the antenna/base unit)
*/

extern "C" __declspec(dllimport)
int set_parameters(unsigned int buffer[5]);
/*
This function

int set_parameters(unsigned int buffer[5]);

is called to set the gain/slope/delay values for the radar
the buffer bits are defined as follows

buffer[n] - (d31,d30,...d0)

the buffer contents are defined as follows


buffer[0]	-
d31 is the initialize radar parameters bit
0	-	normal operation; only any chage in parameter is sent down
1	-	sents all parameters down to the radar
d7-d0 are the Delay D/A value (range - 0 to 255)
buffer[1]	-	Time Base Select (range - 0 to 4095)
buffer[2]	-

d7-d5 are the Cable Delay bits(corresponding to
Dealy_2,Delay_1,Delay_0
bits)
d4 is the Frame Rate control bit
0	- 254 Hz
1	- 127 Hz

buffer[3]	-	Analog Gain (range - 0 to 127)

buffer[4] specifies the control bits as follows

d31-d23	- not used
d22		-
d21		-	Single Antenna Gain bit 1
d20		-	Single Antenna Gain bit 0
d19		-	Differential Antenna Gain bit 1
d18		-	Differential Antenna Gain bit 0
d17		-	Stop Updating SPI
d16		-	PRF1
d15		-	PRF0
00	-	65 khz
01	-	130 khz
10	-	512 khz
11	-	1 MHz
d14		-	not used(internally used for Frame Rate sel - derived from buffer[2] bit d4)
d13		-	A/D Averaging on
d12		-	not used
d11		-	A/D Self Calibration on
d10
- d0	-	not used


The function returns a status code. the bit definitions are as follows

d0		-	Device offline
d1		-	Write queue full(the application should retry later)

*/

extern "C" __declspec(dllimport)
int write_special(unsigned int *buffer, int howmany, int action, char * str_name);

/*
This function

int write_special(unsigned int *buffer,int howmany,int action,char * str_name);

is called for Radar Dongle Board Firmware/FPGA updation

buffer[32768] hold the code/fpga bit stream
howmany specifies the size
action = 0 => FPGA Configuration Update
action = 1 => Write Alternate Program Code
str_name[30] specifies the Version/Date string associated with the code/fpga data

Note: The Dongle/Micro Board will have identical copies of
the "factory or original" Program and the Alternate Program Code
when manufacured.

The Dongle Board runs the "Alternate Program Code" firmware when powerd up.
The Dongle Board runs the "factory or original" firmware when
powered up with the Dongle optical cable looped back! This is indented to
be a fall back position.

*/


/*
This function

int kill_thread(void);

is called by the user application to kill the acquisition thread in csirousb.dll.

this causes the thread to close the usb file handles (if valid) and then return;
*/
extern "C" __declspec(dllimport)
int kill_thread(void);
