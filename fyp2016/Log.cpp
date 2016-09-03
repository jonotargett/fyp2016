#include "Log.h"

Verbosity Log::v = LOG_ALL;
Logger Log::e;
Logger Log::i;
Logger Log::d;
//HRTimer Log::hrt;
std::chrono::time_point<std::chrono::system_clock> Log::start;
bool Log::repostTime = true;
std::streambuf* Log::orig_buf;
std::stringstream Log::stream;
std::vector<std::string> Log::lines;

Log::Log() {
	start = std::chrono::system_clock::now();
	//e = Logger(start, 0xEC);
	//i = Logger(start, 0x0A);
	//d = Logger(start, 0x0F);
	e = Logger(start, 0xCE);
	i = Logger(start, 0x0A);
	d = Logger(start, 0x02);

	e.setStringStream(&stream);
	i.setStringStream(&stream);
	d.setStringStream(&stream);

	orig_buf = cout.rdbuf();
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


void Log::suppressCout(bool suppressed) {
	/*
	if (suppressed) {
		cout.rdbuf(0);
		cerr.rdbuf(0);
		clog.rdbuf(0);


		std::wcerr.rdbuf(0);
		std::wcout.rdbuf(0);
		std::wclog.rdbuf(0);

	}
	else {
		cout.rdbuf(orig_buf);
	}
	*/
}


void Log::process() {
	if (stream.str() != "") {
		lines.push_back(stream.str());
		stream.clear();//clear any bits set
		stream.str(std::string());
	}
}

std::string Log::getLineFromBack(unsigned int i) {


	//process the stringstream into lines (all of them)
	//stream.clear();
	//stream.seekg(0, stream.beg);

	//std::string output;

	//while (getline(stream, output,'\n')) {
	//	lines.push_back(output);
	//}

	//get last 10 lines from vector
	/*
	int len = 0;
	int offset = 0;
	for (unsigned int i = lines.size() - 10; i < lines.size(); i++) {
		len += lines.at(i).length() + 1;
	}
	char* cstring = new char[len + 1];

	for (unsigned int i = lines.size() - 10; i < lines.size(); i++) {
		cstring[offset] = '\n';
		std::strcpy((char*)(cstring + offset + 1), lines.at(i).c_str());
		offset += lines.at(i).length();
	}
	*/

	//get n-ith line from vector
	return lines.at(lines.size() - i - 1);

}