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

	// initialise the surface to the clear color
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, BG_CLEAR_R, BG_CLEAR_G, BG_CLEAR_B);

		// initialise the centreline
		if (drawCentreLine == true) {
			if (i > (image->w*image->h / 2) && i < (image->w*image->h / 2) + (image->w)) {
				// centerline colour is greyish
				pixels[i] = SDL_MapRGB(image->format, CENTRE_LINE_R, CENTRE_LINE_G, CENTRE_LINE_B);
			}
		}
	}
	//top and bottom borderline
	for (int i = 0; i < image->w; i++) {
		pixels[i] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
		pixels[image->w * image->h - i] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
	}
	// left and right borderline
	for (int j = 0; j < image->h; j++) {
		pixels[j*image->w] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
		pixels[j*image->w + image->w - 1] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
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
		pixels[j*width + offset] = SDL_MapRGB(image->format, BG_CLEAR_R, BG_CLEAR_G, BG_CLEAR_B);
		// red update line
		pixels[j*width + offset + 1] = SDL_MapRGB(image->format, UPDATE_LINE_R, UPDATE_LINE_G, UPDATE_LINE_B);
		int pos = (int)(0 + (height - 0)*((val - yMax) / (yMin - yMax)));


		if (j == height / 2 && drawCentreLine == true) {
			// grey centreline
			pixels[j*width + offset] = SDL_MapRGB(image->format, CENTRE_LINE_R, CENTRE_LINE_G, CENTRE_LINE_B);
		}
		else if ((j < pos && j > height / 2) || (j > pos && j < height / 2)) {
			pixels[j*width + offset] = SDL_MapRGB(image->format, COLOR2_R, COLOR2_G, COLOR2_B);
		}

		if (j == pos) {
			// yellow position
			pixels[j*width + offset] = SDL_MapRGB(image->format, COLOR3_R, COLOR3_G, COLOR3_B);
		}
	}

	pixels[0 * width + offset] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
	pixels[(height - 1)*width + offset] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);

	/*
	//top and bottom borderline
	for (int i = 0; i < image->w; i++) {
		pixels[i] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
		pixels[image->w * image->h - i] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
	}
	*/
	
	if(offset == 0 || offset == width - 1) {
		for (int j = 0; j < image->h; j++) {
			pixels[j*width] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
			pixels[j*width + width - 1] = SDL_MapRGB(image->format, BORDER_R, BORDER_G, BORDER_B);
		}
	}
}
