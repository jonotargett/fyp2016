#include "Graph.h"



Graph::Graph(int w, int h, double min, double max)
{
	width = w;
	height = h;
	yMin = min;
	yMax = max;

	offset = 0;
	vals = new double[width];

	image = SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	pixels = (Uint32*)image->pixels;

	// initialise the surface to black
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
	}
}


Graph::~Graph()
{
	delete vals;
	vals = 0;
}

SDL_Surface* Graph::getTexture() {
	return image;
}

void Graph::post(double val) {
	//store the data
	if (offset >= width)
		offset = 0;

	vals[offset] = val;
	offset++;

	//refresh the texture
	for (int j = 0; j < height; j++) {

		pixels[j*image->w + offset] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
		pixels[j*image->w + offset + 1] = SDL_MapRGB(image->format, 0xFF, 0x00, 0x00);

		int pos = 0 + (height - 0)*((val-yMax)/(yMin-yMax));

		if (j == pos) {
			pixels[j*image->w + offset] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0x00);
		}
	}
}
