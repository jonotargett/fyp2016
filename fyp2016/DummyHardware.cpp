#include "DummyHardware.h"



DummyHardware::DummyHardware()
{
}


DummyHardware::~DummyHardware()
{
}


bool DummyHardware::initialise() {
	// TODO: init whatever
	// start comms with the serial device


	// return true once everything is fixed
	std::cout << "DummyHardware initialised." << std::endl;
	return false;
}

bool DummyHardware::updateLoop() {
	// loop continuously
	
	return true;
}