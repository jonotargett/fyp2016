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
	texture->createBlank(textureWidth, textureHeight);

	texture->setAsRenderTarget();

	//clear screen
	SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(Window::renderer);

	int drawScale = 80;
	int focusX = 60;
	int focusY = 100;

	for (int i = 0; i < (int)ns->getPath().size() - 1; i++) {

		double x1Loc = ns->getPath().at(i)->x;
		double y1Loc = ns->getPath().at(i)->y;
		double x2Loc = ns->getPath().at(i+1)->x;
		double y2Loc = ns->getPath().at(i+1)->y;

		// transformed (x,y) locations for scale, computers inverted y coordinate, and focus point
		double x1transform = x1Loc * drawScale - focusX*drawScale + textureWidth/2;
		double y1transform = y1Loc * drawScale * -1 + textureHeight + focusY*drawScale - textureHeight / 2;
		double x2transform = x2Loc * drawScale - focusX*drawScale + textureWidth / 2;
		double y2transform = y2Loc * drawScale * -1 + textureHeight + focusY*drawScale - textureHeight / 2;

		SDL_SetRenderDrawColor(Window::renderer, 0xFF, 0xFF, 0x00, 0xFF);
		SDL_RenderDrawLine(Window::renderer, (int)x1transform, (int)y1transform, (int)x2transform, (int)y2transform);
		SDL_SetRenderDrawColor(Window::renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawPoint(Window::renderer, (int)x1transform, (int)y1transform);
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
	texture->render(0, 0);
}



SDL_Texture* VirtualPlatform::getTexture() {
	return texture->getTexture();
}
