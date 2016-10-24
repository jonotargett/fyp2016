#pragma once

#include <algorithm>
#include "SDL\SDL.h"

#include "Log.h"

#include "Bscan.h"
#include "SimpleTexture.h"
#include "Point.h"

enum RenderPane {
	PANE_TOPLEFT,
	PANE_TOPRIGHT,
	PANE_BOTTOMLEFT,
	PANE_BOTTOMRIGHT,
	PANE_ALL,
	PANE_NULL
};

class Window
{
public:
	Window();
	~Window();


	bool shouldQuit();
	void showWindow(bool);
	void clearWindow(RenderPane);

	void handleEvents();
	void update(SDL_Texture*, RenderPane);
	void update(SDL_Texture*, Point);
	void present();
	SDL_Renderer* getRenderer();

private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Event event;

	SimpleTexture* dontPanic;

	int windowWidth;
	int windowHeight;

	bool quit;
	bool visible;
};

