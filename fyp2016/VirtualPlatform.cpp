#include "VirtualPlatform.h"



VirtualPlatform::VirtualPlatform()
{
	
}
VirtualPlatform::~VirtualPlatform()
{

}

bool VirtualPlatform::initialise(NavigationSystem* nav, SDL_Renderer* r) {
	ns = nav;
	texture = new SimpleTexture(r);
	texture->createBlank(textureWidth, textureHeight);
	return true;
}

void VirtualPlatform::drawTexture() {
	texture->setAsRenderTarget();

	//clear screen
	SDL_SetRenderDrawColor(texture->getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(texture->getRenderer());

	SDL_SetRenderDrawColor(texture->getRenderer(), 0xFF, 0xFF, 0x00, 0xFF);
	int drawScale = 20;
	int focusX = 45;
	int focusY = 85;
	for (int i = 0; i < (int)ns->getPath().size() - 1; i++) {
		SDL_RenderDrawLine(texture->getRenderer(), ns->getPath().at(i)->x * drawScale - focusX*drawScale, ns->getPath().at(i)->y * drawScale * -1 + textureHeight + focusY*drawScale, ns->getPath().at(i+1)->x * drawScale - focusX*drawScale, ns->getPath().at(i+1)->y * drawScale * -1 + textureHeight + focusY*drawScale);
	}
	SDL_SetRenderDrawColor(texture->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
	for (int i = 0; i < (int)ns->getPath().size(); i++) {
		SDL_RenderDrawPoint(texture->getRenderer(), ns->getPath().at(i)->x * drawScale - focusX*drawScale, ns->getPath().at(i)->y * drawScale * -1 + textureHeight + focusY*drawScale);
	}
	
	/*
	//render red filled quad
	SDL_Rect fillRect = { 0, 0, 10, 10 };
	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(Window::renderer, &fillRect);
	*/

	//reset render target
	SDL_SetRenderTarget(texture->getRenderer(), NULL);
}



SDL_Texture* VirtualPlatform::getTexture() {
	return texture->getTexture();
}
