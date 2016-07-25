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

/*
	Draws path to texture for the given drawScale and focus point defined within the function.
*/
void VirtualPlatform::drawTexture() {
	texture->setAsRenderTarget();

	//clear screen
	SDL_SetRenderDrawColor(texture->getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(texture->getRenderer());

	int drawScale = 80;
	int focusX = 60;
	int focusY = 100;

	for (int i = 0; i < (int)ns->getPath().size() - 1; i++) {

		double x1Loc = ns->getPath().at(i)->x;
		double y1Loc = ns->getPath().at(i)->y;
		double x2Loc = ns->getPath().at(i + 1)->x;
		double y2Loc = ns->getPath().at(i + 1)->y;

		// transformed (x,y) locations for scale, computers inverted y coordinate, and focus point
		double x1transform = x1Loc * drawScale - focusX*drawScale + textureWidth / 2;
		double y1transform = y1Loc * drawScale * -1 + textureHeight + focusY*drawScale - textureHeight / 2;
		double x2transform = x2Loc * drawScale - focusX*drawScale + textureWidth / 2;
		double y2transform = y2Loc * drawScale * -1 + textureHeight + focusY*drawScale - textureHeight / 2;

		SDL_SetRenderDrawColor(texture->getRenderer(), 0xFF, 0xFF, 0x00, 0xFF);
		SDL_RenderDrawLine(texture->getRenderer(), (int)x1transform, (int)y1transform, (int)x2transform, (int)y2transform);
		SDL_SetRenderDrawColor(texture->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawPoint(texture->getRenderer(), (int)x1transform, (int)y1transform);
	}

	SDL_SetRenderTarget(texture->getRenderer(), NULL);
}



SDL_Texture* VirtualPlatform::retrieveImage() {
	return texture->getTexture();
}
