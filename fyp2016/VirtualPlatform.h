#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"

class VirtualPlatform
{
private:

public:
	SimpleTexture* texture;
	VirtualPlatform();
	~VirtualPlatform();

	void updateNew();
	SDL_Texture* getTexture();
};

