#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "NavigationSystem.h"

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

};

