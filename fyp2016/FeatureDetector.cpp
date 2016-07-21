#include "FeatureDetector.h"
#include "Window.h"
#include "SimpleTexture.h"

FeatureDetector::FeatureDetector(HardwareInterface* interf) : hwi(interf)
{
}


FeatureDetector::~FeatureDetector()
{
	SDL_FreeSurface(image);
	image = NULL;
}

bool FeatureDetector::initialise() {

	bool success;

	gpr = new GPR();

	success = gpr->initialise();
	if (!success) return false;


	return true;
}


bool FeatureDetector::loadScan() {

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

	SDL_FreeSurface(image);
	image = NULL;
	image = SDL_CreateRGBSurface(0, 1000, 500, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	Uint32 *pixels = (Uint32*)image->pixels;

	// initialise the surface to black
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
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

				int h = v / 256;
				int l = v - h * 256;

				pixels[j*image->w + i] = SDL_MapRGB(image->format, h, h, h);
			}
		}
	}

	Log::d << "Visual scan image created." << std::endl;

	return true;
}


SDL_Surface* FeatureDetector::retrieveImage() {
	SimpleTexture texture = SimpleTexture();
	texture.createBlank(60, 60);

	//set self as render target
	texture.setAsRenderTarget();
	//clear screen
	SDL_SetRenderDrawColor(Window::renderer, 0x00, 0x00, 0xFF, 0xFF);
	SDL_RenderClear(Window::renderer);
	//render red filled quad
	SDL_Rect fillRect = {30, 30, 10, 10 };
	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(Window::renderer, &fillRect);
	//reset render target
	SDL_SetRenderTarget(Window::renderer, NULL);

	texture.render(80, 80);

	return image;
}