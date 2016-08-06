
#include "Overlord.h"
#include "Log.h"

using namespace std;

int main(int argc, char **argv) {


	Log l = Log();
	Log::setVerbosity(LOG_ALL);

	Overlord* main = new Overlord();

	main->initialise();
	main->run();


	return 0;
}