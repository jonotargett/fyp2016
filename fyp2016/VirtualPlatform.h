#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "NavigationSystem.h"

class VirtualPlatform
{
private:
	NavigationSystem* ns;

public:
	VirtualPlatform();
	~VirtualPlatform();

	bool initialise(NavigationSystem*);

	void drawTexture();
	void renderTexture();
	SDL_Texture* getTexture();
	
	SimpleTexture* texture;
};

