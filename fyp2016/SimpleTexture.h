#pragma once

#include "SDL\SDL.h"


class SimpleTexture
{
private:
	SDL_Texture* texture;
	SDL_Renderer* renderer;

	int width;
	int height;
public:
	SimpleTexture(SDL_Renderer* r);
	~SimpleTexture();
	
	void createBlank(int width, int height);
	void freeSimpleTexture();

	void setColor(Uint8 red, Uint8 green, Uint8 blue);
	void setAlpha(Uint8 alpha);
	void setAsRenderTarget();

	SDL_Texture* getTexture();
	SDL_Renderer* getRenderer();
	int getWidth();
	int getHeight();
	
};