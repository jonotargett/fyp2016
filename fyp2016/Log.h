#pragma once

#include <iostream>
#include <istream>

class Log {
private:
	static bool suppressed;
public:
	static void suppress(bool b) { suppressed = b; }
	// TODO
};