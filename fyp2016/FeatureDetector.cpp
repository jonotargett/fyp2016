#include "FeatureDetector.h"
#include "Window.h"
#include "SimpleTexture.h"

FeatureDetector::FeatureDetector(HardwareInterface* interf, SDL_Renderer* r) : hwi(interf), renderer(r)
{
}


FeatureDetector::~FeatureDetector()
{
	SDL_DestroyTexture(texture);
	texture = NULL;
}

bool FeatureDetector::initialise() {

	bool success;

	gpr = new GPR();

	success = gpr->initialise();
	if (!success) return false;


	scan = gpr->getBscan(GPR_DIFFERENTIAL);

	return true;
}

bool FeatureDetector::runScan() {
	bool received = false;

	while (!received) {
		received = gpr->getData();
	}

	return true;
}

bool FeatureDetector::loadScan() {

	delete scan;
	scan = new Bscan();
	Bscan* empty = new Bscan();

	/*
	FILENAMES:

	01011329.DAT	- empty
	01011330.DAT	- empty
	01011348.DAT	- empty
	01041445.DAT	- ST-AT(1)-flat-at0.5cm Pepsi-perp-at0.5cm
	01031453.DAT	- filledPepsi-perp-at0.5cm Pepsi-flat(upright)-at0.5cm
	01021000.DAT	- PVC10x15-para-at0.5cm SS10x05-at0.5cm
	*/

	empty->load("C:/Users/Jono/Documents/Visual Studio 2015/Projects/fyp2016/Debug/01011329.DAT");
	Log::i << "empty scan loaded." << std::endl;
	scan->load("C:/Users/Jono/Documents/Visual Studio 2015/Projects/fyp2016/Debug/01031453.DAT");
	Log::i << "feature scan loaded." << std::endl;

	Ascan* normal = scan->produceNormal(scan->length());
	Log::i << "background noise identified." << std::endl;
	scan->normalise(normal);
	Log::i << "feature scan normalised for background noise." << std::endl;

	return true;
}

bool FeatureDetector::createImage(Visual displayMode) {

	SDL_DestroyTexture(texture);
	texture = NULL;

	
	image = SDL_CreateRGBSurface(0, 1000, 500, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	Uint32 *pixels = (Uint32*)image->pixels;

	// initialise the surface to blue
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0xFF);
	}

	// if a scan is paired to this window, push it to the screen
	if (scan != NULL) {

		int cols = std::min(image->w, scan->length());

		for (int i = 0; i < cols; i++) {
			Ascan* a = scan->element(i);
			int rows = std::min(image->h, a->getSize());


			for (int j = 0; j < rows; j++) {

				int v;
				if (displayMode == DISPLAY_RAW) {
					v = a->getIndex(j);
				}
				if (displayMode == DISPLAY_KERNEL) {
					v = scan->Kernel(3, i, j);
				}

				//int h = v / 256;
				//int l = v - h * 256;

				int16_t n = (int16_t)(v);

				uint8_t h = ((int)(v) / 256);
				uint8_t l = ((int8_t)v);

				pixels[j*image->w + i] = SDL_MapRGB(image->format, h, l, n);
			}
		}


		Log::d << "Visual scan image created." << std::endl; 
		texture = SDL_CreateTextureFromSurface(renderer, image);

		if (texture == NULL) {
			Log::e << "Could not create texture from scan image. " << SDL_GetError() << endl;
		}
	}
	else {
		Log::d << "Scan is NULL" << endl;
	}

	SDL_FreeSurface(image);

	return true;
}


SDL_Texture* FeatureDetector::retrieveImage() {
	return texture;
}