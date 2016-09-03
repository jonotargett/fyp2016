#pragma once

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <chrono>
#include <vector>

//#include "HRTimer.h"
#include "Logger.h"

using std::endl;
using std::cout;
using std::cerr;
using std::clog;

enum Verbosity {
	LOG_ALL,
	LOG_INFORMATIVE,
	LOG_ERROR,
	LOG_NONE
};



class Log {
private:
	static Verbosity v;
	static std::streambuf* orig_buf;
	static std::stringstream stream;
	static std::vector<std::string> lines;

public:
	static bool repostTime;
	static Logger e;	// error;
	static Logger i;	// informative;
	static Logger d;	// debug;
	//static HRTimer hrt;
	static std::chrono::time_point<std::chrono::system_clock> start;
	
	Log();
	static Verbosity getVerbosity();
	static void setVerbosity(Verbosity verbosity);
	static void suppressCout(bool suppressed);

	static void process();
	static std::string getLineFromBack(unsigned int);
};

