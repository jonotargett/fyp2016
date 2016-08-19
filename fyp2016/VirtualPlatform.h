#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "NavigationSystem.h"
#include "QuadBike.h"
#include "SDL\SDL_ttf.h"
#include "Graph.h"

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

	Graph velocityGraph = Graph(360, 76, -2, 2, true);
	Graph steerGraph = Graph(360, 76, -27, 27, true);
	Graph gearGraph = Graph(360, 76, -2, 2, false);
	Graph throttleGraph = Graph(360, 76, -25, 25, true);
	
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
	double focusX = 0;
	double focusY = 0;
	unsigned int currentPathPoint = 0;
	double timer = 0;
	double desiredVelocity = 0;
	bool landmineDetected = false;
	int pathTravDir = 1;

	QuadBike quad;

	Point transform(Point p);
	void setupFont();
	void drawText(std::string textToRender, int x, int y);
	void updateDynamics();
};

