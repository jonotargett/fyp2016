#pragma once

#ifdef _WIN32
#define WINDOWS 1
#else
#define WINDOWS 0
#endif


#include <iostream>
#include <Windows.h>
#include "HRTimer.h"

class Logger {
private:
	bool suppressed;
	short color;
	HRTimer* timer;

public:

	Logger() { timer = NULL; suppressed = false; color = 0x0a; }
	Logger(HRTimer* t) : timer(t) { suppressed = false; color = 0x0a; }
	Logger(HRTimer* t, short col) : timer(t), color(col) { suppressed = false; }
	void suppress(bool b) { suppressed = b; }
	bool isSuppressed() { return suppressed; }


	template<typename T>
	const Logger& operator<<(const T& v) const;

	Logger const& operator<<(std::ostream& (*F)(std::ostream&)) const;
};

#include "Log.h"

template<typename T>
inline const Logger& Logger::operator<<(const T& v) const {

	if (!suppressed) {

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);

		char buf[20];
		sprintf_s(buf, 20, "%12.6f", timer->getElapsedTimeSeconds());

		if (Log::repostTime)
			std::cout << "[" << buf << "] " << v;
		else
			std::cout << v;

		Log::repostTime = false;
	}
	return *this;
}

