#include "VirtualPlatform.h"



VirtualPlatform::VirtualPlatform()
{
	texture = new SimpleTexture();
	

}

void VirtualPlatform::updateNew() {
	texture->createBlank(60, 60);

	//set self as render target
	texture->setAsRenderTarget();
	//clear screen
	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0xFF, 0x00, 0xFF);
	SDL_RenderClear(Window::renderer);
	//render red filled quad
	SDL_Rect fillRect = { 30, 30, 10, 10 };
	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(Window::renderer, &fillRect);
	//reset render target
	SDL_SetRenderTarget(Window::renderer, NULL);

	texture->render(20, 20, 100, 100, 0);
	
}


VirtualPlatform::~VirtualPlatform()
{

}

SDL_Texture* VirtualPlatform::getTexture() {
	return texture->getTexture();
}
