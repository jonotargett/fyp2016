#include "Window.h"



Window::Window()
{
	quit = false;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "SDL could not be initialised. " << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							1250, 850, SDL_WINDOW_SHOWN);

	surface = SDL_GetWindowSurface(window);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));

	SDL_UpdateWindowSurface(window);

	// -----------------------------//

	

	image = SDL_CreateRGBSurface(0, 1200, 800, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

}


Window::~Window()
{
	SDL_FreeSurface(surface);
	surface = NULL;
	SDL_FreeSurface(image);
	image = NULL;

	SDL_DestroyWindow(window);
	window = NULL;
}


void Window::update() {

	Uint32 *pixels = (Uint32*)image->pixels;

	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0xFF, 0xFF, 0x00);
	}

	if (scan != NULL) {

		int cols = std::min(image->w, scan->length());

		for (int i = 0; i < cols; i++) {
			Ascan* a = scan->element(i);
			int rows = std::min(image->h, a->getSize());


			for (int j = 0; j < rows; j++) {

				int v = a->getIndex(j);
				int h = v / 256;
				int l = v - h*256;

				pixels[j*image->w + i] = SDL_MapRGB(image->format, h, h, h);
			}
			
		}

	}


	SDL_BlitSurface(image, NULL, surface, NULL);

	SDL_UpdateWindowSurface(window);

}


void Window::handleEvents() {
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			quit = true;
		}
	}

	SDL_Delay(10);

	//update();
}


bool Window::shouldQuit() {
	return quit;
}


void Window::setBscan(Bscan* newScan) {

	scan = newScan;
}