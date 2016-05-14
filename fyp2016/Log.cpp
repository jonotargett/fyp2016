#include "Log.h"

Verbosity Log::v = LOG_ALL;
Logger Log::e;
Logger Log::i;
Logger Log::d;
//HRTimer Log::hrt;
std::chrono::time_point<std::chrono::system_clock> Log::start;
bool Log::repostTime = true;


Log::Log() {
	//hrt.startTimer();
	start = std::chrono::system_clock::now();
	e = Logger(start, 0xEC);
	i = Logger(start, 0x0A);
	d = Logger(start, 0x0F);
}

Verbosity Log::getVerbosity() {
	return v;
}

void Log::setVerbosity(Verbosity verbosity) {
	v = verbosity;
	switch (v) {
	case LOG_NONE:
		e.suppress(true);
		i.suppress(true);
		d.suppress(true);
		break;
	case LOG_ERROR:
		e.suppress(false);
		i.suppress(true);
		d.suppress(true);
		break;
	case LOG_INFORMATIVE:
		e.suppress(false);
		i.suppress(false);
		d.suppress(true);
		break;
	case LOG_ALL:
	default:
		e.suppress(false);
		i.suppress(false);
		d.suppress(false);
		break;
	}
}