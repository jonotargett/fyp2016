#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "NavigationSystem.h"
#include "QuadBike.h"

class VirtualPlatform
{
public:
	VirtualPlatform();
	~VirtualPlatform();

	bool initialise(NavigationSystem*, SDL_Renderer*);

	void drawTexture();
	SDL_Texture* retrieveImage();
	
private:
	NavigationSystem* ns;
	SimpleTexture* texture;

	int textureWidth;
	int textureHeight;

	int drawScale = 1;
	int focusX = 0;
	int focusY = 0;

	QuadBike quad;

	Point transform(Point p);
};

