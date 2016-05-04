#pragma once

#include <iostream>
#include <algorithm>
#include "SDL\SDL.h"
#include "Bscan.h"

#define SCREEN_WIDTH 800;
#define SCREEN_HEIGHT 600;


class Window
{
private:
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Surface* image;

	SDL_Event event;


	Bscan* scan;


	bool quit;

public:
	Window();
	~Window();

	bool shouldQuit();

	void handleEvents();
	void update();

	void setBscan(Bscan* newScan);
};

