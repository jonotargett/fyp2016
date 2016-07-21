#include "SimpleTexture.h"

SimpleTexture::SimpleTexture()
{
	//Initialize
	texture = NULL;
	width = 0;
	height = 0;
}

SimpleTexture::~SimpleTexture()
{
	free();
}

SDL_Texture* SimpleTexture::getTexture() {
	return texture;
}

void SimpleTexture::createBlank(int w, int h)
{
	texture = SDL_CreateTexture(Window::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
	width = w;
	height = h;
}

void SimpleTexture::free()
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

void SimpleTexture::render(int x, int y)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = {x, y, width, height};

	//Render to screen
	SDL_RenderCopy(Window::renderer, texture, NULL, &renderQuad);
	SDL_RenderPresent(Window::renderer);
}

void SimpleTexture::setAsRenderTarget()
{
	SDL_SetRenderTarget(Window::renderer, texture);
}

int SimpleTexture::getWidth()
{
	return width;
}

int SimpleTexture::getHeight()
{
	return height;
}