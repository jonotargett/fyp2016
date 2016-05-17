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