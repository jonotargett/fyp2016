#pragma once

#include <algorithm>
#include "SDL\SDL.h"

#include "Log.h"

#include "Bscan.h"


class Window
{
private:
	static SDL_Window* window;
	static SDL_Surface* surface;

	static SDL_Event event;

	static bool quit;
	static bool visible;

public:
	Window();
	~Window();

	static bool shouldQuit();
	static void showWindow(bool);

	static void handleEvents();


	static void update(SDL_Surface*);

};

