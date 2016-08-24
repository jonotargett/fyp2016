#include "VirtualPlatform.h"


VirtualPlatform::VirtualPlatform()
{

}
VirtualPlatform::~VirtualPlatform()
{

}

bool VirtualPlatform::initialise(HardwareInterface* hwi, NavigationSystem* nav, DriveController* dc, SDL_Renderer* r) {
	hw = (DummyHardware*) hwi;
	sc = (SimpleController*) dc;
	ns = nav;
	mainCanvas = new SimpleTexture(r);

	textureWidth = 1200;
	textureHeight = 600;

	mainCanvas->createBlank(textureWidth, textureHeight);

	quadTexture = new SimpleTexture(r);
	quadTexture->loadImage("quadBikeImage.png");
	wheelTexture = new SimpleTexture(r);
	wheelTexture->loadImage("quadWheelImage.png");
	sensorTexture = new SimpleTexture(r);
	sensorTexture->loadImage("sensorImage.png");

	setupFont();

	drawScale = 80;
	focusX = 4;
	focusY = -2;

	return true;
}

void VirtualPlatform::update() {
	velocityGraph.post(hw->getRealVelocity());
	steerGraph.post(hw->getRealSteeringAngle() * 180 / PI);
	gearGraph.post(hw->getRealGear());
	throttleGraph.post(hw->getRealThrottlePercentage());

	SDL_PumpEvents();
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		sc->landMineDetected();
	}
}


/*
Draws path to texture for the given drawScale and focus point defined within the function.
*/
void VirtualPlatform::redrawTexture() {

	mainCanvas->setAsRenderTarget();

	//clear screen
	SDL_SetRenderDrawColor(mainCanvas->getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(mainCanvas->getRenderer());

	// drawing the path in this for loop
	for (int i = 0; i < (int)ns->getPath().size() - 1; i++) {

		Point loc1 = Point(ns->getPath().at(i)->x, ns->getPath().at(i)->y);
		Point loc2 = Point(ns->getPath().at(i + 1)->x, ns->getPath().at(i + 1)->y);

		// transformed (x,y) locations for drawing to screen (scale, computers inverted y coordinate, and focus point)
		Point loc1transf = transform(loc1);
		Point loc2transf = transform(loc2);

		SDL_SetRenderDrawColor(mainCanvas->getRenderer(), 0xCC, 0xCC, 0x00, 0xFF);
		SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)loc1transf.x, (int)loc1transf.y, (int)loc2transf.x, (int)loc2transf.y);
		SDL_SetRenderDrawColor(mainCanvas->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)loc1transf.x, (int)loc1transf.y);
	}

	// drawing crosshairs over the focus point
	SDL_SetRenderDrawColor(mainCanvas->getRenderer(), 0x88, 0x88, 0x88, 0xFF);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), textureWidth / 2 - 10, textureHeight / 2 - 10, textureWidth / 2 + 10, textureHeight / 2 + 10);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), textureWidth / 2 - 10, textureHeight / 2 + 10, textureWidth / 2 + 10, textureHeight / 2 - 10);

	Point quadLoc = hw->getRealPosition();
	double heading = hw->getRealAbsoluteHeading();
	
	// Ackermann steering, inside wheel is sharper than outside wheel
	double leftWheelAngle = atan(hw->wheelBase / (hw->wheelBase / tan(hw->getRealSteeringAngle()) - hw->width / 2));
	double rightWheelAngle = atan(hw->wheelBase / (hw->wheelBase / tan(hw->getRealSteeringAngle()) + hw->width / 2));

	// drawing the quadbike wheels
	SDL_Rect leftWheelRect = { (int)transform(quadLoc + getLWheel()).x, (int)transform(quadLoc + getLWheel()).y, (int)(hw->wheelWidth * drawScale), (int)(hw->wheelRadius * 2 * drawScale) };
	SDL_Rect rightWheelRect = { (int)transform(quadLoc + getRWheel()).x,(int)transform(quadLoc + getRWheel()).y, (int)(hw->wheelWidth * drawScale), (int)(hw->wheelRadius * 2 * drawScale) };
	SDL_RenderCopyEx(mainCanvas->getRenderer(), wheelTexture->getTexture(), NULL, &leftWheelRect, (hw->getRealAbsoluteHeading() + leftWheelAngle) * 180 / PI, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mainCanvas->getRenderer(), wheelTexture->getTexture(), NULL, &rightWheelRect, (hw->getRealAbsoluteHeading() + rightWheelAngle) * 180 / PI, NULL, SDL_FLIP_NONE);

	// drawing the sensor mount
	// TODO(Harry): Magic numbers. defining it here isnt good enough. pull it out, create a variable
	// define it there. then use the variable name here. I know that the sensor thing is variable
	// width elsewhere in the code so this should reflect those changes
	double sensorFactor = sensorTexture->getHeight() / 3; // divide by 3 because 3m wide
	SDL_Rect sensorRect = { (int)transform(quadLoc + getSensorTopLeft()).x, (int)transform(quadLoc + getSensorTopLeft()).y, (int)(sensorTexture->getWidth() * drawScale / sensorFactor / 1.25), (int)(sensorTexture->getHeight() * drawScale / sensorFactor) };
	SDL_Point sensorCenter = { 0,0 };
	SDL_RenderCopyEx(mainCanvas->getRenderer(), sensorTexture->getTexture(), NULL, &sensorRect, hw->getRealAbsoluteHeading() * 180 / PI - 90, &sensorCenter, SDL_FLIP_NONE);

	// drawing the quadbike png image
	SDL_Point rotationCenter = { (int)(hw->width / 2), (int)(hw->length - hw->wheelBase) };
	SDL_Rect quadRect = { (int)transform(quadLoc + getFrontL()).x, (int)transform(quadLoc + getFrontL()).y, (int)(hw->width * drawScale), (int)(hw->length * drawScale) };
	SDL_RenderCopyEx(mainCanvas->getRenderer(), quadTexture->getTexture(), NULL, &quadRect, hw->getRealAbsoluteHeading() * 180 / PI, &rotationCenter, SDL_FLIP_NONE);

	// drawing the quadbike outline
	/*SDL_SetRenderDrawColor(mainCanvas->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + getRearL()).x, (int)transform(quadLoc + getRearL()).y,
	(int)transform(quadLoc + getRearR()).x, (int)transform(quadLoc + getRearR()).y);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + getRearR()).x, (int)transform(quadLoc + getRearR()).y,
	(int)transform(quadLoc + getFrontR()).x, (int)transform(quadLoc + getFrontR()).y);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + getFrontR()).x, (int)transform(quadLoc + getFrontR()).y,
	(int)transform(quadLoc + getFrontL()).x, (int)transform(quadLoc + getFrontL()).y);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + getFrontL()).x, (int)transform(quadLoc + getFrontL()).y,
	(int)transform(quadLoc + getRearL()).x, (int)transform(quadLoc + getRearL()).y);*/

	// point at quads local (0,0)
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x + 1, (int)transform(quadLoc).y);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y + 1);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y - 1);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x - 1, (int)transform(quadLoc).y);

	/*//rear wheel (line):
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(Point(quadLoc.x + getRearC().x + sin(heading + getSteerAng())*hw->wheelRadii,0)).x,
	(int)transform(Point(0, quadLoc.y + getRearC().y + cos(heading + getSteerAng())*hw->wheelRadii)).y,
	(int)transform(Point(quadLoc.x + getRearC().x - sin(heading + getSteerAng())*hw->wheelRadii, 0)).x,
	(int)transform(Point(0, quadLoc.y + getRearC().y - cos(heading + getSteerAng())*hw->wheelRadii)).y);
	*/

	// rendering text
	std::string titleText;

	titleText = "Heading: ";
	titleText += std::to_string((int)(hw->getRealAbsoluteHeading() * 180 / PI));
	titleText += " degrees";
	drawText(titleText, 840, 420);

	std::string vel = std::to_string(abs(hw->getRealVelocity()));
	vel.erase(3, 99);
	titleText = "Speed: ";
	titleText += vel;
	titleText += " m/s";
	drawText(titleText, 840, 76);

	titleText = "Throttle: ";
	titleText += std::to_string((int)hw->getRealThrottlePercentage());
	titleText += " %";
	drawText(titleText, 840, 376);

	titleText = "Gear: ";
	if (hw->getRealGear() == 1) titleText += "Drive";
	if (hw->getRealGear() == 0) titleText += "Neutral";
	if (hw->getRealGear() == -1) titleText += "Reverse";
	drawText(titleText, 840, 276);

	titleText = "Brakes: ";
	if (hw->getRealBrake()) titleText += "Applied";
	if (!hw->getRealBrake()) titleText += "Released";
	drawText(titleText, 840, 440);

	titleText = "Steer Angle";
	std::string stang = std::to_string((int)abs(hw->getRealSteeringAngle() * 180 / 3.1416));
	titleText = "Steer Angle: ";
	titleText += stang;
	drawText(titleText, 840, 176);

	SimpleTexture graph1 = SimpleTexture(mainCanvas->getRenderer());
	graph1.loadFromSurface(velocityGraph.retrieveImage());
	SDL_Rect destRect1 = { 840, 0, 360, 76 };

	SimpleTexture graph2 = SimpleTexture(mainCanvas->getRenderer());
	graph2.loadFromSurface(steerGraph.retrieveImage());
	SDL_Rect destRect2 = { 840, 100, 360, 76 };

	SimpleTexture graph3 = SimpleTexture(mainCanvas->getRenderer());
	graph3.loadFromSurface(gearGraph.retrieveImage());
	SDL_Rect destRect3 = { 840, 200, 360, 76 };

	SimpleTexture graph4 = SimpleTexture(mainCanvas->getRenderer());
	graph4.loadFromSurface(throttleGraph.retrieveImage());
	SDL_Rect destRect4 = { 840, 300, 360, 76 };

	SDL_RenderCopy(mainCanvas->getRenderer(), graph1.getTexture(), NULL, &destRect1);
	SDL_RenderCopy(mainCanvas->getRenderer(), graph2.getTexture(), NULL, &destRect2);
	SDL_RenderCopy(mainCanvas->getRenderer(), graph3.getTexture(), NULL, &destRect3);
	SDL_RenderCopy(mainCanvas->getRenderer(), graph4.getTexture(), NULL, &destRect4);

	SDL_SetRenderTarget(mainCanvas->getRenderer(), NULL);
}

Point VirtualPlatform::transform(Point p) {
	//transformed(x, y) locations for scale, computers inverted y coordinate, and focus point
	Point t;
	t.x = p.x * drawScale - focusX*drawScale + textureWidth / 2;
	t.y = p.y * drawScale * -1 + textureHeight + focusY*drawScale - textureHeight / 2;
	return t;
}

SDL_Texture* VirtualPlatform::retrieveImage() {
	return mainCanvas->getTexture();
}

void VirtualPlatform::drawText(std::string textToRender, int x, int y) {
	SDL_Color textColor = { 0, 0, 0, 255 };
	SDL_Surface* textSurface = TTF_RenderText_Blended(standardFont, textToRender.c_str(), textColor);
	SDL_Texture* mTexture = SDL_CreateTextureFromSurface(mainCanvas->getRenderer(), textSurface);
	SDL_FreeSurface(textSurface);

	SDL_Rect renderQuad = { x, y, 0, 0 };
	SDL_QueryTexture(mTexture, NULL, NULL, &renderQuad.w, &renderQuad.h);

	SDL_RenderCopy(mainCanvas->getRenderer(), mTexture, NULL, &renderQuad);

	SDL_DestroyTexture(mTexture);
}

void VirtualPlatform::setupFont() {
	if (TTF_Init() == -1) {
		Log::e << " Failed to initialise TTF : " << SDL_GetError() << endl;
	}
	std::string fontName = "OpenSans-Regular.ttf";
	standardFont = TTF_OpenFont(fontName.c_str(), 12);
}

Point VirtualPlatform::getRearL() {
	double heading = hw->getAbsoluteHeading();
	Point rearLeft;
	rearLeft.x = -(hw->overHang + hw->wheelBase)*sin(heading) - (hw->width / 2) * cos(heading);
	rearLeft.y = -(hw->overHang + hw->wheelBase)*cos(heading) + (hw->width / 2) * sin(heading);
	return rearLeft;
}
Point VirtualPlatform::getRearR() {
	double heading = hw->getAbsoluteHeading();
	Point rearRight;
	rearRight.x = -(hw->overHang + hw->wheelBase)*sin(heading) + (hw->width / 2) * cos(heading);
	rearRight.y = -(hw->overHang + hw->wheelBase)*cos(heading) - (hw->width / 2) * sin(heading);
	return rearRight;
}
Point VirtualPlatform::getFrontL() {
	double heading = hw->getAbsoluteHeading();
	Point frontLeft;
	frontLeft.x = hw->overHang * sin(heading) - (hw->width / 2) * cos(heading);
	frontLeft.y = hw->overHang * cos(heading) + (hw->width / 2) * sin(heading);
	return frontLeft;
}
Point VirtualPlatform::getFrontR() {
	double heading = hw->getAbsoluteHeading();
	Point frontRight;
	frontRight.x = hw->overHang * sin(heading) + (hw->width / 2) * cos(heading);
	frontRight.y = hw->overHang * cos(heading) - (hw->width / 2) * sin(heading);
	return frontRight;
}
Point VirtualPlatform::getRearC() {
	double heading = hw->getAbsoluteHeading();
	Point rearCenter;
	rearCenter.x = -(hw->wheelBase)*sin(heading);
	rearCenter.y = -(hw->wheelBase)*cos(heading);
	return rearCenter;
}

Point VirtualPlatform::getRWheel() {
	double heading = hw->getAbsoluteHeading();
	Point rearRight;
	rearRight.x = -(hw->wheelBase)*sin(heading) + (hw->width / 2 - hw->wheelWidth / 2) * cos(heading) - hw->wheelWidth / 2;
	rearRight.y = -(hw->wheelBase)*cos(heading) - (hw->width / 2 - hw->wheelWidth / 2) * sin(heading) + hw->wheelRadius;
	return rearRight;
}

Point VirtualPlatform::getLWheel() {
	double heading = hw->getAbsoluteHeading();
	Point rearLeft;
	rearLeft.x = -(hw->wheelBase)*sin(heading) - (hw->width / 2 - hw->wheelWidth / 2) * cos(heading) - hw->wheelWidth / 2;
	rearLeft.y = -(hw->wheelBase)*cos(heading) + (hw->width / 2 - hw->wheelWidth / 2) * sin(heading) + hw->wheelRadius;
	return rearLeft;
}

Point VirtualPlatform::getSensorTopLeft() {
	double heading = hw->getAbsoluteHeading();
	Point frontLeft;
	frontLeft.x = hw->overHang * sin(heading) - (1.5) * cos(heading);
	frontLeft.y = hw->overHang * cos(heading) + (1.5) * sin(heading);
	return frontLeft;
}