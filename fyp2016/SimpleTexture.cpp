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

void SimpleTexture::loadFromSurface(SDL_Surface* s) {
	texture = SDL_CreateTextureFromSurface(renderer, s);
}

void SimpleTexture::loadImage(std::string path) {
	//initialise image loading
	IMG_Init(IMG_INIT_PNG);
	SDL_Surface* loadedImage = IMG_Load(path.c_str());
	texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
	SDL_FreeSurface(loadedImage);
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

void SimpleTexture::setAsRenderTarget()
{
	SDL_SetRenderTarget(renderer, texture);
}

int SimpleTexture::getWidth()
{
	SDL_QueryTexture(texture, NULL, NULL, &width, &height);
	return width;
}

int SimpleTexture::getHeight()
{
	SDL_QueryTexture(texture, NULL, NULL, &width, &height);
	return height;
}