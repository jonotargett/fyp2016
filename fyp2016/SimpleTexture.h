#pragma once

#include "SDL\SDL.h"
#include "SDL\SDL_image.h"
#include <iostream>


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
	void loadImage(std::string path);
	void loadFromSurface(SDL_Surface* s);
	void freeSimpleTexture();

	void setAsRenderTarget();

	SDL_Texture* getTexture();
	SDL_Renderer* getRenderer();
	int getWidth();
	int getHeight();
	
};