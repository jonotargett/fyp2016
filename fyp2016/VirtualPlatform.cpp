#include "VirtualPlatform.h"



VirtualPlatform::VirtualPlatform()
{
	texture = new SimpleTexture();
}
VirtualPlatform::~VirtualPlatform()
{

}

bool VirtualPlatform::initialise(NavigationSystem* nav) {
	ns = nav;
	return true;
}

void VirtualPlatform::drawTexture() {
	texture->createBlank(600, 400);

	texture->setAsRenderTarget();

	//clear screen
	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(Window::renderer);

	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0xFF, 0x00, 0xFF);
	int scale = 4;
	for (int i = 0; i < ns->getPath().size() - 1; i++) {
		SDL_RenderDrawLine(Window::renderer, ns->getPath().at(i)->x * scale, ns->getPath().at(i)->y * scale * -1 + 400, ns->getPath().at(i+1)->x * scale, ns->getPath().at(i+1)->y * scale * -1 + 400);
	}
	SDL_SetRenderDrawColor(Window::renderer, 0x00, 0x00, 0x00, 0xFF);
	for (int i = 0; i < ns->getPath().size(); i++) {	
		SDL_RenderDrawPoint(Window::renderer, ns->getPath().at(i)->x * scale, ns->getPath().at(i)->y * scale * -1 + 400);
	}
	
	/*
	//render red filled quad
	SDL_Rect fillRect = { 0, 0, 10, 10 };
	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(Window::renderer, &fillRect);
	*/

	//reset render target
	SDL_SetRenderTarget(Window::renderer, NULL);
}


void VirtualPlatform::renderTexture() {
	texture->render(20, 20);
}



SDL_Texture* VirtualPlatform::getTexture() {
	return texture->getTexture();
}
