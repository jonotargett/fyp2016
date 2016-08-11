#include "Graph.h"



Graph::Graph(int w, int h, double min, double max, bool drawCentre)
{
	width = w;
	height = h;
	yMin = min;
	yMax = max;
	drawCentreLine = drawCentre;

	offset = 0;
	vals = new double[width];

	image = SDL_CreateRGBSurface(0, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	pixels = (Uint32*)image->pixels;

	// initialise the surface to black
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);

		// initialise the centreline
		if (drawCentreLine == true) {
			if (i > (image->w*image->h / 2) && i < (image->w*image->h / 2) + (image->w)) {
				pixels[i] = SDL_MapRGB(image->format, 0x88, 0x88, 0x88);
			}
		}
	}
}


Graph::~Graph()
{
	delete vals;
	vals = 0;
}

SDL_Surface* Graph::retrieveImage() {
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

		if (j == height / 2 && drawCentreLine == true) {
			pixels[j*image->w + offset] = SDL_MapRGB(image->format, 0x88, 0x88, 0x88);
		}

		int pos = (int)(0 + (height - 0)*((val-yMax)/(yMin-yMax)));

		if (j == pos) {
			pixels[j*image->w + offset] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0x00);
		}
	}
}
