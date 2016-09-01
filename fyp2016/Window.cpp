#include "Window.h"
#include "SimpleTexture.h"
#include "SDL\SDL_opengl.h"



Window::Window()
{	
	// aspect ratio 16:9
	// * 80 for  1280 x 720
	// * 100 for 1600 x 900
	// * 120 for 1920 x 1080
	windowWidth = 16 * 100;
	windowHeight = 9 * 100;

	quit = false;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		Log::e << "SDL could not be initialised. " 
			<< SDL_GetError() << std::endl;

		quit = true;
		return;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

	window = SDL_CreateWindow(
		"FYP 2099 Control Program", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		windowWidth, windowHeight, SDL_WINDOW_HIDDEN);

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

	dontPanic = new SimpleTexture(renderer);
	dontPanic->loadImage("dont-panic.jpg");

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

void Window::clearWindow(RenderPane pane) {
	switch (pane) {
	case PANE_ALL:
		SDL_RenderClear(renderer);
		break;
	default:
		break;
	}
	
}


void Window::update(SDL_Texture* newImage, RenderPane pane) {

	int w, h;
	SDL_QueryTexture(newImage, NULL, NULL, &w, &h);
	SDL_Rect destination;
	destination.x = 0;
	destination.y = 0;
	destination.w = windowWidth / 2;
	destination.h = windowHeight / 2;
	
	switch (pane) {
	case PANE_TOPLEFT:
		destination.x = 0;
		destination.y = 0;
		break;
	case PANE_TOPRIGHT:
		destination.x = windowWidth / 2;
		destination.y = 0;
		break;
	case PANE_BOTTOMLEFT:
		destination.x = 0;
		destination.y = windowHeight / 2;
		break;
	case PANE_BOTTOMRIGHT:
		destination.x = windowWidth / 2;
		destination.y = windowHeight / 2;
		break;
	default:
		destination.w = w;
		destination.h = h;
	}

	// this is donw in clearWindow() now
	//SDL_RenderClear(renderer);


	SDL_SetRenderTarget(renderer, NULL);
	if (newImage == NULL) newImage = dontPanic->getTexture();
	SDL_RenderCopy(renderer, newImage, NULL, &destination);
	//SDL_RenderPresent(renderer);

	Log::d << "Window frame changed." << std::endl;
}


void Window::present() {
	SDL_RenderPresent(renderer);
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
