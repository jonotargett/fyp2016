#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "NavigationSystem.h"
#include "QuadBike.h"
#include "SDL\SDL_ttf.h"
#include <sstream>

class VirtualPlatform
{
public:
	VirtualPlatform();
	~VirtualPlatform();

	bool initialise(NavigationSystem*, SDL_Renderer*);
	void update();
	void drawTexture();
	SDL_Texture* retrieveImage();
	void setDesiredVelocity();
	
private:
	NavigationSystem* ns;
	SimpleTexture* mainCanvas;
	SimpleTexture* quadTexture;
	SimpleTexture* wheelTexture;
	SimpleTexture* sensorTexture;
	TTF_Font* standardFont;

	int textureWidth;
	int textureHeight;

	int drawScale = 1;
	int focusX = 0;
	int focusY = 0;
	int currentPathPoint = 0;
	double timer = 0;
	double desiredVelocity = 0;

	QuadBike quad;

	Point transform(Point p);
	void setupFont();
	void VirtualPlatform::drawText(std::string textToRender, int x, int y);
};

