#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "SDL\SDL_ttf.h"
#include "Graph.h"
#include "HardwareInterface.h"
#include "DummyHardware.h"
#include "NavigationSystem.h"
#include "QuadBike.h"

#include <sstream>

class VirtualPlatform
{
public:
	VirtualPlatform();
	~VirtualPlatform();

	bool initialise(HardwareInterface*, NavigationSystem*, SDL_Renderer*);
	void update();
	void redrawTexture();
	SDL_Texture* retrieveImage();

	Graph velocityGraph = Graph(360, 76, -2, 2, true);
	Graph steerGraph = Graph(360, 76, -27, 27, true);
	Graph gearGraph = Graph(360, 76, -2, 2, false);
	Graph throttleGraph = Graph(360, 76, -25, 25, true);

private:
	DummyHardware* hw;
	NavigationSystem* ns;

	SimpleTexture* mainCanvas;
	SimpleTexture* quadTexture;
	SimpleTexture* wheelTexture;
	SimpleTexture* sensorTexture;
	TTF_Font* standardFont;


	QuadBike quad;

	int textureWidth;
	int textureHeight;

	int drawScale = 1;
	double focusX = 0;
	double focusY = 0;

	Point transform(Point p);
	void setupFont();
	void drawText(std::string textToRender, int x, int y);
};

