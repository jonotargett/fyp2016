#pragma once

/*
Thread helper library from http://www.bogotobogo.com/cplusplus/multithreading_win32.php
*/

class Runnable {
public:
	virtual void* run() = 0;
	virtual ~Runnable() = 0;
};
