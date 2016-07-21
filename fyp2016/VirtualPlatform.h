#pragma once

#include "SDL\SDL.h"

class VirtualPlatform
{
private:
	SDL_Surface* image;
	Uint32* pixels;
	int width;
	int height;

	double yMax;
	double yMin;
public:
	VirtualPlatform();
	~VirtualPlatform();

	SDL_Surface* getTexture();
};

