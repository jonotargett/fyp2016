#pragma once

#include "SDL\SDL.h"
#include "Window.h"
#include "SimpleTexture.h"
#include "SDL\SDL_ttf.h"
#include "Graph.h"
#include "HardwareInterface.h"
#include "DummyHardware.h"
#include "NavigationSystem.h"
#include "DriveController.h"
#include "SimpleController.h"

#include <sstream>

class VirtualPlatform
{
public:
	VirtualPlatform();
	~VirtualPlatform();

	bool initialise(HardwareInterface*, NavigationSystem*, DriveController*, SDL_Renderer*);
	void update();
	void redrawSimulationTexture();
	void redrawGraphTexture();
	SDL_Texture* retrieveSimulationImage();
	SDL_Texture* retrieveGraphImage();
	void drawPathToTexture();

private:
	HardwareInterface* hw;
	NavigationSystem* ns;
	SimpleController* sc;
	SimpleTexture* pathCanvas;
	SimpleTexture* simulationCanvas;
	SimpleTexture* graphCanvas;
	SimpleTexture* quadTexture;
	SimpleTexture* wheelTexture;
	SimpleTexture* sensorTexture;
	TTF_Font* standardFont;
	SDL_Surface* textSurface;

	Graph* velocityGraph;
	Graph* steerGraph;
	Graph* gearGraph;
	Graph* throttleGraph;
	int graphWidth;
	int graphHeight;

	int textureWidth;
	int textureHeight;

	double drawScale = 0.2;
	double focusX = 0;
	double focusY = 0;

	Point transform(Point p);

	void setupFont();
	void drawText(std::string textToRender, int x, int y, bool fromEnd = false);


	Point getRearL();
	Point getRearR();
	Point getFrontL();
	Point getFrontR();
	Point getRearC();
	Point getLWheel();
	Point getRWheel();
	Point getSensorTopLeft();
};

