#pragma once

#include <string.h>

#include "SDL\SDL.h"
#include "Window.h"

class SimpleTexture
{
public:
	SimpleTexture();
	~SimpleTexture();
	
	SDL_Texture* getTexture();
	void createBlank(int width, int height);
	void free();
	void setColor(Uint8 red, Uint8 green, Uint8 blue);
	void setAlpha(Uint8 alpha);
	void setAsRenderTarget();
	void render(int x, int y,  int centerX, int centerY, double angle, SDL_Rect* clip = NULL);
	SDL_Renderer* renderer;

	int getWidth();
	int getHeight();

private:
	
	SDL_Texture* texture;
	
	int width;
	int height;
};