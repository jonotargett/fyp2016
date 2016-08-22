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
	Graph(int, int, double, double, bool);
	~Graph();

	void post(double);
	bool drawCentreLine = true;
	SDL_Surface* retrieveImage();
};

