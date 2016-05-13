#pragma once

#include <algorithm>
#include "SDL\SDL.h"

#include "Log.h"

#include "Bscan.h"


class Window
{
private:
	SDL_Window* window;
	SDL_Surface* surface;

	SDL_Event event;

	bool quit;
	bool visible;

public:
	Window();
	~Window();

	bool shouldQuit();
	void showWindow(bool);

	void handleEvents();


	void update(SDL_Surface*);

};

