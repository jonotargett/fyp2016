#include "Window.h"



Window::Window()
{
	quit = false;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		Log::e << "SDL could not be initialised. " << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow("Signal Processing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							1200, 600, SDL_WINDOW_HIDDEN);

	surface = SDL_GetWindowSurface(window);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));

	SDL_UpdateWindowSurface(window);

	Log::i << "SDL Window created." << std::endl;

	// -----------------------------//

}


Window::~Window()
{
	SDL_FreeSurface(surface);
	surface = NULL;

	SDL_DestroyWindow(window);
	window = NULL;
}


void Window::showWindow(bool b) {
	visible = b;

	if (visible) {
		SDL_ShowWindow(window);
		Log::d << "window made visible" << endl;
	}
	else {
		SDL_HideWindow(window);
		Log::d << "window hidden" << endl;

	}
}


void Window::update(SDL_Surface* newImage) {

	SDL_BlitSurface(newImage, NULL, surface, NULL);

	SDL_UpdateWindowSurface(window);

	Log::d << "Window frame changed." << std::endl;
}


void Window::handleEvents() {
	while (SDL_PollEvent(&event) != 0) {
		if (event.type == SDL_QUIT) {
			quit = true;
		}
	}

	SDL_Delay(10);
}


bool Window::shouldQuit() {
	return quit;
}

