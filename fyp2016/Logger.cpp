#include "Logger.h"

#include "Log.h"





Logger const& Logger::operator<<(std::ostream& (*F)(std::ostream&)) const {

	Log::process();
	Log::repostTime = true;

	if (!suppressed) {
		F(std::cout);
	}
	return *this;
}