#include "VirtualPlatform.h"



VirtualPlatform::VirtualPlatform()
{
	width = 100;
	height = 100;
	yMin = 0;
	yMax = 10;

	image = SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	pixels = (Uint32*)image->pixels;

	// initialise the surface to black
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0x00);
	}
}


VirtualPlatform::~VirtualPlatform()
{
}

SDL_Surface* VirtualPlatform::getTexture() {
	return image;
}
