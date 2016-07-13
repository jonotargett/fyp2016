#include "SimpleNavigator.h"



SimpleNavigator::SimpleNavigator()
{
}


SimpleNavigator::~SimpleNavigator()
{
}


bool SimpleNavigator::initialise(DriveController* controller, HardwareInterface* hardware) {
	dc = controller;
	hwi = hardware;
	
	// subdivide() here for testing purposes at the moment
	subdivide();

	Log::i << "Navigator initialised." << std::endl;
	return true;
}

void SimpleNavigator::clear() {

}

void SimpleNavigator::setPath(std::vector<Point*>) {

}

void SimpleNavigator::addPoint(Point p) {
	Point* np = new Point(p.x, p.y);

	path.push_back(np);
}

bool SimpleNavigator::subdivide() {

	//filling path with dummy points for testing purposes:
	Point p = Point(30, 10);
	addPoint(p);
	p = Point(20, 50);
	addPoint(p);
	p = Point(60, 80);
	addPoint(p);
	p = Point(80, 20);
	addPoint(p);

	Log::i << "Path subdivision starting" << endl;
	Log::i << "Path size: " << path.size() << endl;

	std::vector<Point*> subdividedPath;	

	Point directionVector = Point(path.at(1)->x - path.at(0)->x, path.at(1)->y - path.at(0)->y);
	Log::i << "Lenght: " << directionVector.x << endl;


	Log::i << "Path subdivision Completed" << endl;
	
	


	return false;
}

bool SimpleNavigator::startPath() {

	updater = new std::thread(&SimpleNavigator::loop, this);
	return true;
}


void SimpleNavigator::loop() {
	endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> seconds = endTime - startTime;

	Point curPos = hwi->getPosition();

	if (path.size() <= 0)
		return;

	Point* nextPos = path.at(0);
	Point dif = Point(nextPos->x, nextPos->y);
	dif = Point(dif.x - curPos.x, dif.y - curPos.y);
	double dist = sqrt(dif.x * dif.x + dif.y*dif.y);

	while (true) {
		curPos = hwi->getPosition();
		dif = Point(nextPos->x, nextPos->y);
		dif = Point(dif.x - curPos.x, dif.y - curPos.y);
		dist = sqrt(dif.x * dif.x + dif.y*dif.y);


		dc->setInputs(0, dist);

		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}

	
}