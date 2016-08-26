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

	// initialise the surface to white
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);

		// initialise the centreline
		if (drawCentreLine == true) {
			if (i > (image->w*image->h / 2) && i < (image->w*image->h / 2) + (image->w)) {
				// centerline colour is greyish
				pixels[i] = SDL_MapRGB(image->format, 0x88, 0x88, 0x88);
			}
		}
	}
	//top and bottom borderline
	for (int i = 0; i < image->w; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
		pixels[image->w * image->h - i] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
	}
	// left and right borderline
	for (int j = 0; j < image->h; j++) {
		pixels[j*image->w] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
		pixels[j*image->w + image->w - 1] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
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

		// black background
		pixels[j*image->w + offset] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
		// red update line
		pixels[j*image->w + offset + 1] = SDL_MapRGB(image->format, 0xFF, 0x00, 0x00);

		if (j == height / 2 && drawCentreLine == true) {
			// grey centreline
			pixels[j*image->w + offset] = SDL_MapRGB(image->format, 0x88, 0x88, 0x88);
		}

		int pos = (int)(0 + (height - 0)*((val-yMax)/(yMin-yMax)));

		if (j == pos) {
			// yellow position
			pixels[j*image->w + offset] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0x00);
		}
	}
	//top and bottom borderline
	for (int i = 0; i < image->w; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
		pixels[image->w * image->h - i] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
	}
	// left and right borderline
	for (int j = 0; j < image->h; j++) {
		pixels[j*image->w] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
		pixels[j*image->w + image->w - 1] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0xFF);
	}
}
