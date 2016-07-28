#pragma once

#ifdef _WIN32
#define TERMINAL_COLORS
#endif


#ifdef TERMINAL_COLORS
#define NOMINMAX
#include <Windows.h>
#endif


#include <iostream>

//#include "HRTimer.h"
#include <chrono>


class Logger {
private:
	bool suppressed;
	short color;
	//HRTimer* timer;
	std::chrono::time_point<std::chrono::system_clock> start;

public:

	Logger() { start = std::chrono::system_clock::now(); suppressed = false; color = 0x0a; }
	Logger(std::chrono::time_point<std::chrono::system_clock> s) : start(s) { suppressed = false; color = 0x0a; }
	Logger(std::chrono::time_point<std::chrono::system_clock> s, short col) : start(s), color(col) { suppressed = false; }
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

#ifdef TERMINAL_COLORS
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
#endif

		std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
		std::chrono::duration<double> seconds = end - start;

		char buf[20];
		sprintf(buf, "%12.6f", seconds.count());

		if (Log::repostTime)
			std::cout << "[" << buf << "]\t" << v;
		else
			std::cout << v;

		Log::repostTime = false;
	}
	return *this;
}

