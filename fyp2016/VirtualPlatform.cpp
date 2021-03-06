#include "VirtualPlatform.h"


VirtualPlatform::VirtualPlatform()
{

}
VirtualPlatform::~VirtualPlatform()
{

}

bool VirtualPlatform::initialise(HardwareInterface* hwi, NavigationSystem* nav, DriveController* dc, SDL_Renderer* r) {
	hw = hwi;
	sc = (SimpleController*) dc;
	ns = nav;

	pathCanvas = new SimpleTexture(r);
	simulationCanvas = new SimpleTexture(r);
	graphCanvas = new SimpleTexture(r);

	// stores the size of the window
	int w, h;
	SDL_GetRendererOutputSize(r, &w, &h);
	// texture width and heights should be half of sceen width and height
	textureWidth = w/2;
	textureHeight = h/2;

	pathCanvas->createBlank(w*2, h*2);
	pathTextureHeight = h;
	pathTextureWidth = w;
	simulationCanvas->createBlank(w, h);
	graphCanvas->createBlank(textureWidth, textureHeight);

	quadTexture = new SimpleTexture(r);
	quadTexture->loadImage("quadBikeImage.png");
	wheelTexture = new SimpleTexture(r);
	wheelTexture->loadImage("quadWheelImage.png");
	sensorTexture = new SimpleTexture(r);
	sensorTexture->loadImage("sensorImage.png");

	graphWidth = textureWidth;
	graphHeight = (textureHeight/4);
	
	velocityGraph = new Graph(graphWidth, graphHeight, -2, 2, true);
	steerGraph = new Graph(graphWidth, graphHeight, -27, 27, true);
	gearGraph = new Graph(graphWidth, graphHeight, -2, 2, false);
	throttleGraph = new Graph(graphWidth, graphHeight, -25, 25, true);

	setupFont();

	drawScale = 70;
	focusX = 0;
	focusY = 9;

	xmin = 0;
	xmax = 0;
	ymin = 0;
	ymax = 0;

	return true;
}

void VirtualPlatform::update() {
	velocityGraph->post(hw->getRealVelocity());
	steerGraph->post(hw->getRealSteeringAngle() * 180 / PI);
	gearGraph->post(hw->getRealGear());
	throttleGraph->post(round(hw->getRealThrottlePercentage()));
	focusX = hw->getRealPosition().x;
	focusY = hw->getRealPosition().y;
}


void VirtualPlatform::redrawGraphTexture() {

	Point quadLoc = hw->getRealPosition();
	double heading = hw->getRealAbsoluteHeading();
	double steerAngle = hw->getRealSteeringAngle();

	graphCanvas->setAsRenderTarget();
	// clear to white
	SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0xEE, 0xEE, 0xF2, 0xFF);
	SDL_RenderClear(simulationCanvas->getRenderer());

	
	SimpleTexture graphVeloc = SimpleTexture(graphCanvas->getRenderer());
	SimpleTexture graphSteer = SimpleTexture(graphCanvas->getRenderer());
	SimpleTexture graphGear = SimpleTexture(graphCanvas->getRenderer());
	SimpleTexture graphThrot = SimpleTexture(graphCanvas->getRenderer());
	
	graphVeloc.loadFromSurface(velocityGraph->retrieveImage());
	graphSteer.loadFromSurface(steerGraph->retrieveImage());
	graphGear.loadFromSurface(gearGraph->retrieveImage());
	graphThrot.loadFromSurface(throttleGraph->retrieveImage());

	SDL_Rect destRectVeloc = { 0, 0, graphWidth, graphHeight };
	SDL_Rect destRectSteer = { 0, (graphHeight), graphWidth, graphHeight };
	SDL_Rect destRectGear = { 0, 2* (graphHeight), graphWidth, graphHeight };
	SDL_Rect destRectThrot = { 0, 3* (graphHeight), graphWidth, graphHeight };

	SDL_RenderCopy(graphCanvas->getRenderer(), graphVeloc.getTexture(), NULL, &destRectVeloc);
	SDL_RenderCopy(graphCanvas->getRenderer(), graphSteer.getTexture(), NULL, &destRectSteer);
	SDL_RenderCopy(graphCanvas->getRenderer(), graphGear.getTexture(), NULL, &destRectGear);
	SDL_RenderCopy(graphCanvas->getRenderer(), graphThrot.getTexture(), NULL, &destRectThrot);

	
	drawText("Velocity", graphWidth - 5, 0 * (graphHeight), true);
	drawText("Steering Angle", graphWidth - 5, 1 * (graphHeight), true);
	drawText("Gear Selection", graphWidth-5, 2 * (graphHeight), true);
	drawText("Throttle Percentage", graphWidth-5, 3 * (graphHeight), true);


	// rendering text
	std::string titleText;

	titleText = "Heading: ";
	titleText += std::to_string((int)(heading * 180 / PI));
	titleText += " degrees";
	drawText(titleText, 840, 420);

	std::string vel = std::to_string(abs(hw->getRealVelocity()));
	vel.erase(3, 99);
	titleText = "Speed: ";
	titleText += vel;
	titleText += " m/s";
	//drawText(titleText, 840, 76);
	drawText(titleText, graphWidth - 5, 1 * (graphHeight) - 18, true);

	titleText = "Throttle: ";
	titleText += std::to_string((int)round(hw->getRealThrottlePercentage()));
	titleText += " %";
	//drawText(titleText, 840, 376);
	drawText(titleText, graphWidth - 5, 4 * (graphHeight) - 18, true);

	titleText = "Gear: ";
	if (hw->getRealGear() == 1) titleText += "Drive";
	if (hw->getRealGear() == 0) titleText += "Neutral";
	if (hw->getRealGear() == -1) titleText += "Reverse";
	//drawText(titleText, 840, 276);
	drawText(titleText, graphWidth - 5, 3 * (graphHeight) - 18, true);

	titleText = "Brake Percentage: ";
	std::string brakePerc = std::to_string((int)abs(hw->getRealBrakePercentage()));
	titleText += brakePerc;
	drawText(titleText, 840, 440);

	titleText = "Steer Angle: ";
	std::string stang = std::to_string((int)round(abs(steerAngle * 180 / PI)));
	titleText += stang;
	titleText += " degrees";
	//drawText(titleText, 840, 176);
	drawText(titleText, graphWidth - 5, 2 * (graphHeight) - 18, true);

	

	SDL_SetRenderTarget(graphCanvas->getRenderer(), NULL);

}

/*
Draws path to texture for the given drawScale and focus point defined within the function.
*/
void VirtualPlatform::drawPathToTexture() {
	xmin = ns->getSubdividedPath()->at(0).x;
	xmax = ns->getSubdividedPath()->at(0).x;
	ymin = ns->getSubdividedPath()->at(0).y;
	ymax = ns->getSubdividedPath()->at(0).y;
	for (int i = 0; i < ns->getSubdividedPath()->size(); i++) {
		xmin = std::min(xmin, ns->getSubdividedPath()->at(i).x);
		xmax = std::max(xmax, ns->getSubdividedPath()->at(i).x);
		ymin = std::min(ymin, ns->getSubdividedPath()->at(i).y);
		ymax = std::max(ymax, ns->getSubdividedPath()->at(i).y);
	}

	double width = xmax - xmin;
	double height = ymax - ymin;

	pathTextureWidth = (int)ceil(width * drawScale);
	pathTextureHeight = (int)ceil(height * drawScale);

	pathCanvas->createBlank(pathTextureWidth, pathTextureHeight);
	pathCanvas->setAsRenderTarget();
	SDL_SetRenderDrawColor(pathCanvas->getRenderer(), 0xDD, 0xFF, 0xDD, 0xFF);
	SDL_RenderClear(pathCanvas->getRenderer());
	
	// drawing the path in this for loop
	for (int i = 0; i < (int)ns->getSubdividedPath()->size() - 1; i++) {
		Point loc1 = Point(-xmin + ns->getSubdividedPath()->at(i).x, ymin + height - ns->getSubdividedPath()->at(i).y);
		Point loc2 = Point(-xmin + ns->getSubdividedPath()->at(i + 1).x, ymin + height - ns->getSubdividedPath()->at(i + 1).y);

		// transformed (x,y) locations for drawing to screen (scale, computers inverted y coordinate, and focus point)
		Point loc1transf = loc1 * drawScale;
		Point loc2transf = loc2 * drawScale;

		SDL_SetRenderDrawColor(pathCanvas->getRenderer(), 0xCC, 0xCC, 0x00, 0xFF);
		SDL_RenderDrawLine(pathCanvas->getRenderer(), (int)loc1transf.x, (int)loc1transf.y, (int)loc2transf.x, (int)loc2transf.y);
		SDL_SetRenderDrawColor(pathCanvas->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderDrawPoint(pathCanvas->getRenderer(), (int)loc1transf.x, (int)loc1transf.y);
	}

	SDL_SetRenderTarget(pathCanvas->getRenderer(), NULL);
}

void VirtualPlatform::redrawSimulationTexture() {
	simulationCanvas->setAsRenderTarget();
	SDL_SetRenderDrawColor(pathCanvas->getRenderer(), 0xDD, 0xFF, 0xDD, 0xFF);
	SDL_RenderClear(simulationCanvas->getRenderer());

	Point quadLoc = hw->getRealPosition();
	double heading = hw->getRealAbsoluteHeading();
	double kalmanHeading = hw->getKalmanHeading();
	double steerAngle = hw->getRealSteeringAngle();

	//SDL_SetRenderTarget(simulationCanvas->getRenderer(), NULL);
	//drawPathToTexture();
	//simulationCanvas->setAsRenderTarget();

	// copy path texture over to this one
	SDL_Rect pathDest = {transform(Point(xmin, ymin)).x, transform(Point(xmin, ymin)).y - pathTextureHeight, pathTextureWidth, pathTextureHeight };
	SDL_RenderCopy(simulationCanvas->getRenderer(), pathCanvas->getTexture(), NULL, &pathDest);
	//t.x = p.x * drawScale - focusX*drawScale + (0.6*textureWidth);
	//t.y = p.y * drawScale * -1 + textureHeight + focusY*drawScale;// +textureHeight / 2;

	// drawing crosshairs over the focus point
	//SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0x88, 0x88, 0x88, 0xFF);
	//SDL_RenderDrawLine(simulationCanvas->getRenderer(), textureWidth / 2 - 10, textureHeight / 2 - 10, textureWidth / 2 + 10, textureHeight / 2 + 10);
	//SDL_RenderDrawLine(simulationCanvas->getRenderer(), textureWidth / 2 - 10, textureHeight / 2 + 10, textureWidth / 2 + 10, textureHeight / 2 - 10);

	// Ackermann steering, inside wheel is sharper than outside wheel
	double leftWheelAngle = atan(hw->wheelBase / (hw->wheelBase / tan(steerAngle) - hw->width / 2));
	double rightWheelAngle = atan(hw->wheelBase / (hw->wheelBase / tan(steerAngle) + hw->width / 2));

	// drawing the quadbike wheels
	SDL_Rect leftWheelRect = { (int)transform(quadLoc + getLWheel()).x, (int)transform(quadLoc + getLWheel()).y, (int)(hw->wheelWidth * drawScale), (int)(hw->wheelRadius * 2 * drawScale) };
	SDL_Rect rightWheelRect = { (int)transform(quadLoc + getRWheel()).x,(int)transform(quadLoc + getRWheel()).y, (int)(hw->wheelWidth * drawScale), (int)(hw->wheelRadius * 2 * drawScale) };
	SDL_RenderCopyEx(simulationCanvas->getRenderer(), wheelTexture->getTexture(), NULL, &leftWheelRect, (heading + leftWheelAngle) * 180 / PI, NULL, SDL_FLIP_NONE);
	SDL_RenderCopyEx(simulationCanvas->getRenderer(), wheelTexture->getTexture(), NULL, &rightWheelRect, (heading + rightWheelAngle) * 180 / PI, NULL, SDL_FLIP_NONE);

	// drawing the sensor mount
	// TODO(Harry): Magic numbers. defining it here isnt good enough. pull it out, create a variable
	// define it there. then use the variable name here. I know that the sensor thing is variable
	// width elsewhere in the code so this should reflect those changes
	double sensorFactor = sensorTexture->getHeight() / 3; // divide by 3 because 3m wide
	SDL_Rect sensorRect = { (int)transform(quadLoc + getSensorTopLeft()).x, (int)transform(quadLoc + getSensorTopLeft()).y, (int)(sensorTexture->getWidth() * drawScale / sensorFactor / 1.25), (int)(sensorTexture->getHeight() * drawScale / sensorFactor) };
	SDL_Point sensorCenter = { 0,0 };
	SDL_RenderCopyEx(simulationCanvas->getRenderer(), sensorTexture->getTexture(), NULL, &sensorRect, heading * 180 / PI - 90, &sensorCenter, SDL_FLIP_NONE);

	// drawing the quadbike png image
	SDL_Point rotationCenter = { (int)(hw->width / 2), (int)(hw->length - hw->wheelBase) };
	SDL_Rect quadRect = { (int)transform(quadLoc + getFrontL()).x, (int)transform(quadLoc + getFrontL()).y, (int)(hw->width * drawScale), (int)(hw->length * drawScale) };
	SDL_RenderCopyEx(simulationCanvas->getRenderer(), quadTexture->getTexture(), NULL, &quadRect, heading * 180 / PI, &rotationCenter, SDL_FLIP_NONE);

	// drawing the quadbike outline
	/*SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc + getRearL()).x, (int)transform(quadLoc + getRearL()).y,
	(int)transform(quadLoc + getRearR()).x, (int)transform(quadLoc + getRearR()).y);
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc + getRearR()).x, (int)transform(quadLoc + getRearR()).y,
	(int)transform(quadLoc + getFrontR()).x, (int)transform(quadLoc + getFrontR()).y);
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc + getFrontR()).x, (int)transform(quadLoc + getFrontR()).y,
	(int)transform(quadLoc + getFrontL()).x, (int)transform(quadLoc + getFrontL()).y);
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc + getFrontL()).x, (int)transform(quadLoc + getFrontL()).y,
	(int)transform(quadLoc + getRearL()).x, (int)transform(quadLoc + getRearL()).y);*/

	// point at quads local (0,0)
	
	SDL_RenderDrawPoint(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y);
	SDL_RenderDrawPoint(simulationCanvas->getRenderer(), (int)transform(quadLoc).x + 1, (int)transform(quadLoc).y);
	SDL_RenderDrawPoint(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y + 1);
	SDL_RenderDrawPoint(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y - 1);
	SDL_RenderDrawPoint(simulationCanvas->getRenderer(), (int)transform(quadLoc).x - 1, (int)transform(quadLoc).y);

	/* //rear wheel (line):
	SDL_RenderDrawLine(mainCanvas->getRenderer(), (int)transform(Point(quadLoc.x + getRearC().x + sin(heading + getSteerAng())*hw->wheelRadii,0)).x,
	(int)transform(Point(0, quadLoc.y + getRearC().y + cos(heading + getSteerAng())*hw->wheelRadii)).y,
	(int)transform(Point(quadLoc.x + getRearC().x - sin(heading + getSteerAng())*hw->wheelRadii, 0)).x,
	(int)transform(Point(0, quadLoc.y + getRearC().y - cos(heading + getSteerAng())*hw->wheelRadii)).y);
	*/

	/*
	// draw the real heading vector:
	SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y, transform(Point(quadLoc.x + 2 * sin(heading), quadLoc.y + 2 * cos(heading))).x, transform(Point(quadLoc.x + 2 * sin(heading), quadLoc.y + 2 * cos(heading))).y);

	*/

	///////////////////////
	////// IMU visual /////
	///////////////////////
	double imuHeading = hw->getImuHeading();
	SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0xFF, 0xAA, 0xFF, 0xFF);
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y, (int)transform(Point(quadLoc.x + 2 * sin(imuHeading), quadLoc.y + 2 * cos(imuHeading))).x, (int)transform(Point(quadLoc.x + 2 * sin(imuHeading), quadLoc.y + 2 * cos(imuHeading))).y);
	
	//location size
	int ptSize = 3; // each side of actual location

	///////////////////////
	// kinematic visual ///
	///////////////////////
	SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0xFF, 0x00, 0x00, 0xFF);
	Point kinematicDrawPos = transform(hw->getKinematicPosition());
	SDL_Rect kinematicRect = { kinematicDrawPos.x - ptSize, kinematicDrawPos.y - ptSize, ptSize * 2, ptSize * 2 };
	SDL_RenderFillRect(simulationCanvas->getRenderer(), &kinematicRect);

	double kinHeading = hw->getKinematicHeading();
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y, transform(Point(quadLoc.x + 2 * sin(kinHeading), quadLoc.y + 2 * cos(kinHeading))).x, transform(Point(quadLoc.x + 2 * sin(kinHeading), quadLoc.y + 2 * cos(kinHeading))).y);

	///////////////////////
	////// gps visual /////
	///////////////////////
	SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0x00, 0x00, 0xFF, 0xFF);
	Point gpsDrawPos = transform(hw->getGpsPosition());
	SDL_Rect gpsRect = { gpsDrawPos.x - ptSize, gpsDrawPos.y - ptSize, ptSize*2, ptSize *2};
	SDL_RenderFillRect(simulationCanvas->getRenderer(), &gpsRect);

	double gpsHeading = hw->getGpsHeading();
	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y, transform(Point(quadLoc.x + 2 * sin(gpsHeading), quadLoc.y + 2 * cos(gpsHeading))).x, transform(Point(quadLoc.x + 2 * sin(gpsHeading), quadLoc.y + 2 * cos(gpsHeading))).y);
	
	///////////////////////
	///// kalman visual ///
	///////////////////////
	SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0x00, 0xAA, 0x00, 0xFF);
	Point kalmanDrawPos = transform(hw->getKalmanPosition());
	SDL_Rect kalmanRect = { kalmanDrawPos.x - ptSize, kalmanDrawPos.y - ptSize, ptSize * 2, ptSize * 2 };
	SDL_RenderFillRect(simulationCanvas->getRenderer(), &kalmanRect);

	SDL_RenderDrawLine(simulationCanvas->getRenderer(), (int)transform(quadLoc).x, (int)transform(quadLoc).y, transform(Point(quadLoc.x + 2 * sin(kalmanHeading), quadLoc.y + 2 * cos(kalmanHeading))).x, transform(Point(quadLoc.x + 2 * sin(kalmanHeading), quadLoc.y + 2 * cos(kalmanHeading))).y);
	
	/////////////////////////
	// currentPoint visual //
	/////////////////////////
	SDL_SetRenderDrawColor(simulationCanvas->getRenderer(), 0xCC, 0x00, 0xCC, 0xFF);
	Point currentTurnPoint = transform(ns->getPoint());
	SDL_Rect curTurnRect = { currentTurnPoint.x - ptSize, currentTurnPoint.y - ptSize, ptSize * 2, ptSize * 2 };
	SDL_RenderFillRect(simulationCanvas->getRenderer(), &curTurnRect);


	SDL_SetRenderTarget(simulationCanvas->getRenderer(), NULL);
}

Point VirtualPlatform::transform(Point p) {
	//transformed(x, y) locations for scale, computers inverted y coordinate, and focus point
	Point t;
	//t.x = p.x * drawScale - focusX*drawScale + textureWidth / 2;
	//t.y = p.y * drawScale * -1 + textureHeight + focusY*drawScale - textureHeight / 2;
	t.x = p.x * drawScale - focusX*drawScale + (0.6*textureWidth);
	t.y = p.y * drawScale * -1 + textureHeight + focusY*drawScale;// +textureHeight / 2;
	return t;
}

SDL_Texture* VirtualPlatform::retrieveSimulationImage() {
	return simulationCanvas->getTexture();
}
SDL_Texture* VirtualPlatform::retrieveGraphImage() {
	return graphCanvas->getTexture();
}

void VirtualPlatform::drawText(std::string textToRender, int x, int y, bool fromEnd) {
	
		SDL_Color textColor = { 0, 0, 0, 255 };
		textSurface = TTF_RenderText_Blended(standardFont, textToRender.c_str(), textColor);
		SDL_Texture* mTexture = SDL_CreateTextureFromSurface(simulationCanvas->getRenderer(), textSurface);

		SDL_FreeSurface(textSurface);

		int width = 0;
		int height = 0;

		SDL_QueryTexture(mTexture, NULL, NULL, &width, &height);

		SDL_Rect renderQuad = { x, y, width, height };
		if (fromEnd) {
			renderQuad.x = renderQuad.x - width;
		}

		SDL_RenderCopy(simulationCanvas->getRenderer(), mTexture, NULL, &renderQuad);

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
	double heading = hw->getRealAbsoluteHeading();
	Point rearLeft;
	rearLeft.x = -(hw->overHang + hw->wheelBase)*sin(heading) - (hw->width / 2) * cos(heading);
	rearLeft.y = -(hw->overHang + hw->wheelBase)*cos(heading) + (hw->width / 2) * sin(heading);
	return rearLeft;
}
Point VirtualPlatform::getRearR() {
	double heading = hw->getRealAbsoluteHeading();
	Point rearRight;
	rearRight.x = -(hw->overHang + hw->wheelBase)*sin(heading) + (hw->width / 2) * cos(heading);
	rearRight.y = -(hw->overHang + hw->wheelBase)*cos(heading) - (hw->width / 2) * sin(heading);
	return rearRight;
}
Point VirtualPlatform::getFrontL() {
	double heading = hw->getRealAbsoluteHeading();
	Point frontLeft;
	frontLeft.x = hw->overHang * sin(heading) - (hw->width / 2) * cos(heading);
	frontLeft.y = hw->overHang * cos(heading) + (hw->width / 2) * sin(heading);
	return frontLeft;
}
Point VirtualPlatform::getFrontR() {
	double heading = hw->getRealAbsoluteHeading();
	Point frontRight;
	frontRight.x = hw->overHang * sin(heading) + (hw->width / 2) * cos(heading);
	frontRight.y = hw->overHang * cos(heading) - (hw->width / 2) * sin(heading);
	return frontRight;
}
Point VirtualPlatform::getRearC() {
	double heading = hw->getRealAbsoluteHeading();
	Point rearCenter;
	rearCenter.x = -(hw->wheelBase)*sin(heading);
	rearCenter.y = -(hw->wheelBase)*cos(heading);
	return rearCenter;
}

Point VirtualPlatform::getRWheel() {
	double heading = hw->getRealAbsoluteHeading();
	Point rearRight;
	rearRight.x = -(hw->wheelBase)*sin(heading) + (hw->width / 2 - hw->wheelWidth / 2) * cos(heading) - hw->wheelWidth / 2;
	rearRight.y = -(hw->wheelBase)*cos(heading) - (hw->width / 2 - hw->wheelWidth / 2) * sin(heading) + hw->wheelRadius;
	return rearRight;
}

Point VirtualPlatform::getLWheel() {
	double heading = hw->getRealAbsoluteHeading();
	Point rearLeft;
	rearLeft.x = -(hw->wheelBase)*sin(heading) - (hw->width / 2 - hw->wheelWidth / 2) * cos(heading) - hw->wheelWidth / 2;
	rearLeft.y = -(hw->wheelBase)*cos(heading) + (hw->width / 2 - hw->wheelWidth / 2) * sin(heading) + hw->wheelRadius;
	return rearLeft;
}

Point VirtualPlatform::getSensorTopLeft() {
	double heading = hw->getRealAbsoluteHeading();
	Point frontLeft;
	frontLeft.x = hw->overHang * sin(heading) - (1.5) * cos(heading);
	frontLeft.y = hw->overHang * cos(heading) + (1.5) * sin(heading);
	return frontLeft;
}