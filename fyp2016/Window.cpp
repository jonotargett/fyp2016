#include "Window.h"



Window::Window()
{
	quit = false;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cout << "SDL could not be initialised. " << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow("Signal Processing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							1200, 800, SDL_WINDOW_SHOWN);

	surface = SDL_GetWindowSurface(window);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));

	SDL_UpdateWindowSurface(window);

	std::cout << "SDL Window created." << std::endl;

	// -----------------------------//

}


Window::~Window()
{
	SDL_FreeSurface(surface);
	surface = NULL;

	SDL_DestroyWindow(window);
	window = NULL;
}


void Window::update(SDL_Surface* newImage) {

	SDL_BlitSurface(newImage, NULL, surface, NULL);

	SDL_UpdateWindowSurface(window);

	std::cout << "Window frame changed." << std::endl;
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

