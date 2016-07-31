#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "NavigationSystem.h"
#include "QuadBike.h"
#include "SDL\SDL_ttf.h"

class VirtualPlatform
{
public:
	VirtualPlatform();
	~VirtualPlatform();

	bool initialise(NavigationSystem*, SDL_Renderer*);
	void update();
	void drawTexture();
	SDL_Texture* retrieveImage();
	
private:
	NavigationSystem* ns;
	SimpleTexture* texture;
	TTF_Font* standardFont;

	int textureWidth;
	int textureHeight;

	int drawScale = 1;
	int focusX = 0;
	int focusY = 0;
	int timer = 0;

	QuadBike quad;

	Point transform(Point p);
	void setupFont();
	void VirtualPlatform::drawText(std::string textToRender, int x, int y);
};

