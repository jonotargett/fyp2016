#include "SimpleTexture.h"

SimpleTexture::SimpleTexture(SDL_Renderer* r) : renderer(r)
{
	//Initialize
	texture = NULL;
	width = 0;
	height = 0;
}

SimpleTexture::~SimpleTexture()
{
	freeSimpleTexture();
}

SDL_Texture* SimpleTexture::getTexture() {
	return texture;
}
SDL_Renderer* SimpleTexture::getRenderer() {
	return renderer;
}

void SimpleTexture::createBlank(int w, int h)
{
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
	width = w;
	height = h;
}

void SimpleTexture::freeSimpleTexture()
{
	//Free texture if it exists
	if (texture != NULL)
	{
		SDL_DestroyTexture(texture);
		texture = NULL;
		width = 0;
		height = 0;
	}
}

void SimpleTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	SDL_SetTextureColorMod(texture, red, green, blue);
}

void SimpleTexture::setAlpha(Uint8 alpha)
{
	SDL_SetTextureAlphaMod(texture, alpha);
}

void SimpleTexture::setAsRenderTarget()
{
	SDL_SetRenderTarget(renderer, texture);
}

int SimpleTexture::getWidth()
{
	return width;
}

int SimpleTexture::getHeight()
{
	return height;
}