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

	textureWidth = 800;
	textureHeight = 600;

	texture->createBlank(textureWidth, textureHeight);

	return true;
}

void VirtualPlatform::update() {
	quad.update();
}

/*
	Draws path to texture for the given drawScale and focus point defined within the function.
*/
void VirtualPlatform::drawTexture() {
	texture->setAsRenderTarget();

	//clear screen
	SDL_SetRenderDrawColor(texture->getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(texture->getRenderer());

	drawScale = 80;
	focusX = 0;
	focusY = 0;
	
	// drawing the path in this for loop
	for (int i = 0; i < (int)ns->getPath().size() - 1; i++) {

		Point loc1 = Point(ns->getPath().at(i)->x, ns->getPath().at(i)->y);
		Point loc2 = Point(ns->getPath().at(i + 1)->x, ns->getPath().at(i + 1)->y);

		// transformed (x,y) locations for drawing to screen (scale, computers inverted y coordinate, and focus point)
		Point loc1transf = transform(loc1);
		Point loc2transf = transform(loc2);

		SDL_SetRenderDrawColor(texture->getRenderer(), 0xCC, 0xCC, 0x00, 0xFF);
		SDL_RenderDrawLine(texture->getRenderer(), (int)loc1transf.x, (int)loc1transf.y, (int)loc2transf.x, (int)loc2transf.y);
		SDL_SetRenderDrawColor(texture->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawPoint(texture->getRenderer(), (int)loc1transf.x, (int)loc1transf.y);
	}

	// drawing crosshairs over the focus point
	SDL_SetRenderDrawColor(texture->getRenderer(), 0x88, 0x88, 0x88, 0xFF);
	SDL_RenderDrawLine(texture->getRenderer(), textureWidth / 2 -10, textureHeight / 2 - 10, textureWidth / 2+10, textureHeight / 2 + 10);
	SDL_RenderDrawLine(texture->getRenderer(), textureWidth / 2 - 10, textureHeight / 2 +10, textureWidth / 2 + 10, textureHeight / 2-10);


	// drawing the quadbike
	Point quadLoc = quad.getLocation();
	SDL_SetRenderDrawColor(texture->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderDrawLine(texture->getRenderer(), (int)transform(quadLoc + quad.getRearL()).x, (int)transform(quadLoc + quad.getRearL()).y,
												(int)transform(quadLoc + quad.getRearR()).x, (int)transform(quadLoc + quad.getRearR()).y);
	SDL_RenderDrawLine(texture->getRenderer(), (int)transform(quadLoc + quad.getRearR()).x, (int)transform(quadLoc + quad.getRearR()).y,
												(int)transform(quadLoc + quad.getFrontR()).x, (int)transform(quadLoc + quad.getFrontR()).y);
	SDL_RenderDrawLine(texture->getRenderer(), (int)transform(quadLoc + quad.getFrontR()).x, (int)transform(quadLoc + quad.getFrontR()).y,
												(int)transform(quadLoc + quad.getFrontL()).x, (int)transform(quadLoc + quad.getFrontL()).y);
	SDL_RenderDrawLine(texture->getRenderer(), (int)transform(quadLoc + quad.getFrontL()).x, (int)transform(quadLoc + quad.getFrontL()).y,
												(int)transform(quadLoc + quad.getRearL()).x, (int)transform(quadLoc + quad.getRearL()).y);
	SDL_SetRenderTarget(texture->getRenderer(), NULL);
}

Point VirtualPlatform::transform(Point p) {
	//transformed(x, y) locations for scale, computers inverted y coordinate, and focus point
	Point t;
	t.x = p.x * drawScale - focusX*drawScale + textureWidth / 2;
	t.y = p.y * drawScale * -1 + textureHeight + focusY*drawScale - textureHeight / 2;
	return t;
}

SDL_Texture* VirtualPlatform::retrieveImage() {
	return texture->getTexture();
}
