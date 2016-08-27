#pragma once

#include "SDL\SDL.h"


#define BG_CLEAR_R 245
#define BG_CLEAR_G 245
#define BG_CLEAR_B 245

#define UPDATE_LINE_R 16
#define UPDATE_LINE_G 121
#define UPDATE_LINE_B 209

#define CENTRE_LINE_R 0xCC
#define CENTRE_LINE_G 0xCE
#define CENTRE_LINE_B 0xDB

#define BORDER_R 0xCC
#define BORDER_G 0xCE
#define BORDER_B 0xDB

#define COLOR1_R 16
#define COLOR1_G 121
#define COLOR1_B 209

#define COLOR2_R 163
#define COLOR2_G 199
#define COLOR2_B 216

#define COLOR3_R 94
#define COLOR3_G 142
#define COLOR3_B 205

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

