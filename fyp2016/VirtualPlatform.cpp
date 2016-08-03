#include "VirtualPlatform.h"


VirtualPlatform::VirtualPlatform()
{
	
}
VirtualPlatform::~VirtualPlatform()
{

}

bool VirtualPlatform::initialise(NavigationSystem* nav, SDL_Renderer* r) {
	ns = nav;
	mainCanvas = new SimpleTexture(r);

	textureWidth = 800;
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
	focusX = 0;
	focusY = -2;

	quad.setState("cruise");

	return true;
}

void VirtualPlatform::update() {

	// find angle between heading and to the next path point
	double angleToPathPoint = -1 * atan2(ns->getPath().at(currentPathPoint)->y - quad.getLocation().y, ns->getPath().at(currentPathPoint)->x - quad.getLocation().x) + 3.14159265 / 2;
	if (angleToPathPoint > 3.141593) angleToPathPoint -= 2 * 3.141593;
	if (angleToPathPoint < -3.141593) angleToPathPoint += 2 * 3.141593;
	double alpha = angleToPathPoint - quad.getHeading();
	double distance = quad.getLocation().getDistanceTo(*ns->getPath().at(currentPathPoint));
	double steerAngleReq = -atan(2 * quad.wheelBase * sin(alpha) / distance);

	if (steerAngleReq > quad.maxSteerAngle) steerAngleReq = quad.maxSteerAngle;
	if (steerAngleReq < -quad.maxSteerAngle) steerAngleReq = -quad.maxSteerAngle;
	quad.setSteerAng(steerAngleReq);


	if (currentPathPoint + 1 >= ns->getPath().size()) {
		// next point doesnt exist
		return;
	}
	
	if (distance > quad.getLocation().getDistanceTo(*ns->getPath().at(currentPathPoint + 1))) {
		if (quad.getState() != "turnInbound") {
			// this means that we need to change direction when the quadbike reaches currentPathPoint (turn inbound?).
			quad.setState("turnInbound");
		}
	}

	// are we going forward (1) or backward (-1)
	int direction;
	if (alpha < -3.1416 / 2 || alpha > 3.1416 / 2) {
		direction = -1;
	}
	else {
		direction = 1;
	}

	if (quad.getState() == "turnInbound") {
		// kinda bad because if quad overshoots it will keep going.
		desiredVelocity = direction * 2 * distance;
		if (abs(desiredVelocity) > quad.cruisesVelocity) 
			desiredVelocity = direction * quad.cruisesVelocity;
		
		if (distance < 0.1) {
			quad.setState("cruise");
			currentPathPoint++;
		}
	}
	else if (quad.getState() == "cruise") {
		desiredVelocity = quad.cruisesVelocity * direction;
		if (distance < 1.2) currentPathPoint++;
	}

	if (abs(quad.getSteerAng() - steerAngleReq) > 3 * 3.1416 / 180) {
		desiredVelocity = 0;
	}

	setDesiredVelocity();
	quad.update();
}

// assumes correct gear is selected
void VirtualPlatform::setDesiredVelocity() {
	if (desiredVelocity == 0) {
		quad.setThrottlePercentage(0);
		quad.setGear(0);
		quad.setBrake(true);
	}
	else if (desiredVelocity > 0) {
		// if we are travelling in the wrong direction
		if (quad.getVelocity() < 0) {
			quad.setThrottlePercentage(0);
			quad.setGear(0);
			quad.setBrake(true);
			return;
		}
		// if we are travelling in the correct direction
		else {
			quad.setGear(1);
			quad.setBrake(false);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (desiredVelocity < quad.getIdleSpeed()) {
			if (quad.getVelocity() < desiredVelocity) {
				quad.setThrottlePercentage(0);
				quad.setBrake(false);
			}
			else {
				quad.setThrottlePercentage(0);
				quad.setBrake(true);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large 
		// difference in actual speed and desired speed
		else {
			if (quad.getVelocity() < desiredVelocity) {
				quad.setThrottlePercentage(quad.getThrottle() + 0.1);
			}
			if (quad.getVelocity() > desiredVelocity) {
				quad.setThrottlePercentage(quad.getThrottle() - 1);
				//quad.setBrake(true);
			}
		}
	}
	else if (desiredVelocity < 0) {
		// if we are travelling in the wrong direction
		if (quad.getVelocity() > 0) {
			quad.setThrottlePercentage(0);
			quad.setGear(0);
			quad.setBrake(true);
			return;
		}
		// if we are travelling in the correct direction
		else {
			quad.setGear(-1);
			quad.setBrake(false);
		}

		// if desiredVelocity is so slow that we need to feather the brakes
		if (abs(desiredVelocity) < quad.getIdleSpeed()) {
			if (quad.getVelocity() < desiredVelocity) {
				quad.setThrottlePercentage(0);
				quad.setBrake(true);
			}
			else {
				quad.setThrottlePercentage(0);
				quad.setBrake(false);
			}
		}
		// otherwise we'll feather the throttle
		// maybe add brakes in here later for a really large
		// difference in actual speed and desired speed
		else {
			//REMEMBER WE'RE IN REVERSE HERE
			if (quad.getVelocity() < desiredVelocity) {
				quad.setThrottlePercentage(quad.getThrottle() - 0.1);
			}
			if (quad.getVelocity() > desiredVelocity) {
				quad.setThrottlePercentage(quad.getThrottle() + 0.1);
			}
		}
	}
}

/*
	Draws path to texture for the given drawScale and focus point defined within the function.
*/
void VirtualPlatform::drawTexture() {

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
	SDL_RenderDrawLine(mainCanvas->getRenderer(), textureWidth / 2 -10, textureHeight / 2 - 10, textureWidth / 2+10, textureHeight / 2 + 10);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), textureWidth / 2 - 10, textureHeight / 2 +10, textureWidth / 2 + 10, textureHeight / 2-10);

	Point quadLoc = quad.getLocation();
	// drawing the quadbike wheels
	SDL_Rect leftWheelRect = { transform(quadLoc + quad.getLWheel()).x, transform(quadLoc + quad.getLWheel()).y, quad.wheelWidth * drawScale, quad.wheelRadii * 2 * drawScale };
	SDL_Rect rightWheelRect = { transform(quadLoc + quad.getRWheel()).x, transform(quadLoc + quad.getRWheel()).y, quad.wheelWidth * drawScale, quad.wheelRadii * 2 * drawScale };
	SDL_RenderCopyEx(mainCanvas->getRenderer(), wheelTexture->getTexture(), NULL, &leftWheelRect, (quad.getHeading() + quad.getSteerAng() + quad.getSteerAng()/5) * 180/3.1416, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(mainCanvas->getRenderer(), wheelTexture->getTexture(), NULL, &rightWheelRect, (quad.getHeading() + quad.getSteerAng() - quad.getSteerAng()/5) * 180 / 3.1416, NULL, SDL_FLIP_NONE);

	// drawing the sensor mount
	SDL_Rect sensorRect = { transform(quadLoc + quad.getSensorTopLeft()).x, transform(quadLoc + quad.getSensorTopLeft()).y, 1.5 * drawScale, 3 * drawScale };
	SDL_Point sensorCenter = { 0,0 };
	SDL_RenderCopyEx(mainCanvas->getRenderer(), sensorTexture->getTexture(), NULL, &sensorRect, quad.getHeading() * 180 / 3.1416 - 90, &sensorCenter, SDL_FLIP_NONE);

	// drawing the quadbike png image
	SDL_Point rotationCenter = { quad.width / 2, quad.length - quad.wheelBase };
	SDL_Rect quadRect = { transform(quadLoc + quad.getFrontL()).x, transform(quadLoc + quad.getFrontL()).y, quad.width * drawScale, quad.length * drawScale };
	SDL_RenderCopyEx(mainCanvas->getRenderer(), quadTexture->getTexture(), NULL, &quadRect, quad.getHeading() * 180/3.1416, &rotationCenter, SDL_FLIP_NONE);
	
	// drawing the quadbike outline
	/*SDL_SetRenderDrawColor(mainCanvas->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + quad.getRearL()).x, (int)transform(quadLoc + quad.getRearL()).y,
												(int)transform(quadLoc + quad.getRearR()).x, (int)transform(quadLoc + quad.getRearR()).y);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + quad.getRearR()).x, (int)transform(quadLoc + quad.getRearR()).y,
												(int)transform(quadLoc + quad.getFrontR()).x, (int)transform(quadLoc + quad.getFrontR()).y);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + quad.getFrontR()).x, (int)transform(quadLoc + quad.getFrontR()).y,
												(int)transform(quadLoc + quad.getFrontL()).x, (int)transform(quadLoc + quad.getFrontL()).y);
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(quadLoc + quad.getFrontL()).x, (int)transform(quadLoc + quad.getFrontL()).y,
												(int)transform(quadLoc + quad.getRearL()).x, (int)transform(quadLoc + quad.getRearL()).y);*/

	// point at quads local (0,0)
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x+1, (int)transform(quadLoc).y);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y+1);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y-1);
	SDL_RenderDrawPoint(mainCanvas->getRenderer(), (int)transform(quadLoc).x-1, (int)transform(quadLoc).y);

	/*//rear wheel (line):
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(Point(quadLoc.x + quad.getRearC().x + sin(quad.getHeading() + quad.getSteerAng())*quad.wheelRadii,0)).x,
												(int)transform(Point(0, quadLoc.y + quad.getRearC().y + cos(quad.getHeading() + quad.getSteerAng())*quad.wheelRadii)).y,
												(int)transform(Point(quadLoc.x + quad.getRearC().x - sin(quad.getHeading() + quad.getSteerAng())*quad.wheelRadii, 0)).x,
												(int)transform(Point(0, quadLoc.y + quad.getRearC().y - cos(quad.getHeading() + quad.getSteerAng())*quad.wheelRadii)).y);
	*/

	// rendering text
	drawText("Hey, you fools!", 10, 10);
	
	std::string titleText;
	titleText = "Velocity:  ";
	titleText += std::to_string(quad.getVelocity());
	titleText += " m/s";
	drawText(titleText, 10, 300);

	titleText = "Heading:  ";
	titleText += std::to_string((int)(quad.getHeading() * 180 / 3.1416));
	titleText += " degrees";
	drawText(titleText, 10, 340);

	titleText = "Throttle:  ";
	titleText += std::to_string((int) quad.getThrottle());
	titleText += " %";
	drawText(titleText, 10, 380);

	titleText = "Rounded Velocity:  ";
	titleText += std::to_string(round(quad.getVelocity() * 10)/10);
	titleText += " m/s";
	drawText(titleText, 10, 420);

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
}

void VirtualPlatform::setupFont() {
	if (TTF_Init() == -1) {
		Log::e << " Failed to initialise TTF : " << SDL_GetError() << endl;
	}
	std::string fontName = "Pacifico.ttf";
	standardFont = TTF_OpenFont(fontName.c_str(), 16);
}