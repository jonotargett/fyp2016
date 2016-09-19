
#include "Overlord.h"
#include "Log.h"
#include "Matrix.h"

using namespace std;

int main(int argc, char **argv) {


	Log l = Log();
	Log::setVerbosity(LOG_ALL);

	Overlord* main = new Overlord();

	main->initialise();
	main->run();



	/*Matrix<double> m1(2, 3);
	for (unsigned int i = 0; i < m1.getRows(); i++) {
		for (unsigned int j = 0; j < m1.getColumns(); j++) {
			m1.put(i, j, i * m1.getColumns() + j + 20);
		}
	}
	cout << m1 << endl;
	
	while (true) {
		
	}*/


	return 0;
}