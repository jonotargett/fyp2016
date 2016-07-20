#pragma once

#include "SDL\SDL.h"

#include "HardwareInterface.h"

class VirtualPlatform
{
private:
	HardwareInterface* hwi;
	SDL_Surface* image;

public:
	VirtualPlatform(HardwareInterface*);
	~VirtualPlatform();

	bool initialise();

	//debug programs
	bool createImage(Visual);
	SDL_Surface* retrieveImage();
};
