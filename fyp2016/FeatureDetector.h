#pragma once

#include <vector>
#include <algorithm>
#include "SDL\SDL.h"

#include "Log.h"

#include "Ascan.h"
#include "Bscan.h"
#include "HardwareInterface.h"
#include "GPR.h"
#include "MD.h"

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

	bool isLandmineDetected();

	bool runScan();

	//debug programs
	bool createImage(Visual);
	SDL_Texture* retrieveImage();
	SDL_Texture* retrieveMDImage();

private:

	float clip(float, float, float);
	void generateColorMap();
	uint8_t* colormap;


	HardwareInterface* hwi;
	GPR* gpr;
	MD* md;

	SDL_Renderer* renderer;
	SDL_Texture* texture;
	SDL_Surface* image;
	Bscan* scan;
	
};

