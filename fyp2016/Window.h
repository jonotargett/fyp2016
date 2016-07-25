#pragma once

#include <algorithm>
#include "SDL\SDL.h"

#include "Log.h"

#include "Bscan.h"


class Window
{
public:
	Window();
	~Window();


	bool shouldQuit();
	void showWindow(bool);
	void clearWindow();

	void handleEvents();
	void update(SDL_Texture*);
	SDL_Renderer* getRenderer();

private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;

	bool quit;
	bool visible;
};

