#pragma once

#include "SDL\SDL.h"

class Graph
{
private:
	SDL_Surface* image;
	Uint32* pixels;
	int width;
	int height;

	double yMax;
	double yMin;
	double* vals;
	int offset;
public:
	Graph(int, int, double, double);
	~Graph();

	void post(double);
	SDL_Surface* retrieveImage();
};

