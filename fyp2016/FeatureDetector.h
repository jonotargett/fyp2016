#pragma once

#include <vector>
#include <algorithm>
#include "SDL\SDL.h"

#include "Log.h"

#include "Ascan.h"
#include "Bscan.h"
#include "HardwareInterface.h"
#include "GPR.h"

enum Visual {
	DISPLAY_RAW,
	DISPLAY_KERNEL
};

class FeatureDetector
{
public:
	FeatureDetector(HardwareInterface*, SDL_Renderer*);
	~FeatureDetector();

	bool initialise();
	bool loadScan();

	bool runScan();

	//debug programs
	bool createImage(Visual);
	SDL_Texture* retrieveImage();

private:
	HardwareInterface* hwi;
	GPR* gpr;

	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Surface* image;
	Bscan* scan;
	
};

