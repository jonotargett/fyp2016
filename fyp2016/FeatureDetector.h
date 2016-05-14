#pragma once

#include <vector>
#include <algorithm>
#include "SDL\SDL.h"

#include "Log.h"

#include "Ascan.h"
#include "Bscan.h"
#include "HardwareInterface.h"

enum Visual {
	DISPLAY_RAW,
	DISPLAY_KERNEL
};

class FeatureDetector
{
private:
	Bscan* scan;
	HardwareInterface* hwi;
	SDL_Surface* image;

public:
	FeatureDetector(HardwareInterface*);
	~FeatureDetector();

	bool initialise();

	//debug programs
	bool createImage(Visual);
	SDL_Surface* retrieveImage();
};

