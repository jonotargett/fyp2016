#include "Window.h"
#include "SimpleTexture.h"


Window::Window()
{
	
	quit = false;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		Log::e << "SDL could not be initialised. " 
			<< SDL_GetError() << std::endl;

		quit = true;
		return;
	}

	window = SDL_CreateWindow(
		"FYP 2099 Control Program", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1200, 600, SDL_WINDOW_HIDDEN);

	if (window == NULL) {
		Log::e << "SDL Window could not be created. "
			<< SDL_GetError() << endl;
		quit = true;
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == NULL) {
		Log::e << "SDL Renderer could not be created. " 
			<< SDL_GetError() << endl;
		quit = true;
		return;
	}

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);



	/*
	surface = SDL_GetWindowSurface(window);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF));

	SDL_UpdateWindowSurface(window);

	Log::i << "SDL Window created." << std::endl;

	// -----------------------------//
	*/

}


Window::~Window()
{
	SDL_DestroyWindow(window);
	window = NULL;

	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	//SDL_DestroyTexture(texture);
	//texture = NULL;
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

void Window::update(SDL_Texture* newImage) {

	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderCopy(renderer, newImage, NULL, NULL);
	SDL_RenderPresent(renderer);

	Log::d << "Window frame changed." << std::endl;
}

SDL_Renderer* Window::getRenderer() {
	return renderer;
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
