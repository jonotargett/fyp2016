#include "Overlord.h"



Overlord::Overlord()
{
	showvp = true;
	initialised = false;
}


Overlord::~Overlord()
{
}


bool Overlord::initialise() {
	Log::i << "--------------------------------------------------------------" << endl;

	Log::i << "-> Creating Window..." << endl;
	window = new Window();
	Log::i << "-> WINDOW DONE" << endl << endl;

	Log::i << "-> Initialising communications..." << endl;
	comms = new Communications(2099);
	comms->initialise();
	comms->setListener(this);
	Log::i << "-> COMMUNICATIONS DONE" << endl << endl;

	Log::i << "-> Initialising hardware interface..." << endl;
	hwi = new QuadInterface();
	hwi->initialise();
	Log::i << "-> HARDWARE INTERFACE DONE" << endl << endl;

	Log::i << "-> Initialising dummuy hardware..." << endl;
	dhwi = new DummyHardware();
	dhwi->initialise();
	Log::i << "-> DUMMY HARDWARE DONE" << endl << endl;
	
	Log::i << "-> Initialising navigation system..." << endl;
	ns = new SimpleNavigator();
	ns->initialise();
	ns->clearPath();

	ns->addPoint(Point(0, 3.5));
	ns->addPoint(Point(3.5, 3.5));
	ns->addPoint(Point(1, -4));
	ns->addPoint(Point(-2, -4));

	// test 1: 0deg
	//ns->addPoint(Point(0, 4));

	// test 2: 20deg
	//ns->addPoint(Point(0, 3.5));
	//ns->addPoint(Point(1.2, 6.3));

	// test 3: 40deg
	//ns->addPoint(Point(0, 3.5));
	//ns->addPoint(Point(2, 6));
	// test 4: 65deg
	//ns->addPoint(Point(0, 3.5));
	//ns->addPoint(Point(3.17, 4.45));

	//test 5: 90deg
	//ns->addPoint(Point(0, 3.5));
	//ns->addPoint(Point(3.5, 3.5));

	// test 6: 120deg
	//ns->addPoint(Point(0, 3.5));
	//ns->addPoint(Point(3.03, 1.25));

	// test 7: realistic 180deg swathe
	//ns->addPoint(Point(0, 6));
	//ns->addPoint(Point(3, 6));
	//ns->addPoint(Point(3, 3));

	// test 8: square
	//ns->addPoint(Point(0, 3));
	//ns->addPoint(Point(3, 3));
	//ns->addPoint(Point(3, 0));
	//ns->addPoint(Point(0, 0));
	

	// realistic swathe example
	/*ns->addPoint(Point(0, 1));
	ns->addPoint(Point(0, 3));
	ns->addPoint(Point(3, 3));
	ns->addPoint(Point(3, -20));
	ns->addPoint(Point(6, -20));
	ns->addPoint(Point(6, 0));*/
	//as
	// example used in 4.5.4 Path subdivision
	/*drawScale = 45;
	focusX = 20;
	focusY = 20;
	ns->addPoint(Point(0, 0));
	ns->addPoint(Point(13, 15));
	ns->addPoint(Point(18.5, 19));
	ns->addPoint(Point(21, 19));
	ns->addPoint(Point(22.2, 21.6));
	ns->addPoint(Point(23.4, 23.2));*/

	// example used in 4.5.3 Turning the platform a specified angle
	/*drawScale = 45;
	focusX = 20;
	focusY = 20;
	ns->addPoint(Point(0, 0));
	ns->addPoint(Point(20, 0));
	ns->addPoint(Point(20, 20));
	ns->addPoint(Point(24, 24));*/

	ns->subdivide(dhwi->getPosition(), (float)dhwi->getAbsoluteHeading());
	Log::i << "-> NAVIGATION SYSTEM DONE" << endl << endl;
	
	Log::i << "-> Initialising drive controller..." << endl;
	dc = new SimpleController();
	dc->initialise(dhwi, ns);
	Log::i << "-> DRIVE CONTROLLER DONE" << endl << endl;

	Log::i << "-> Starting feature detection system..." << endl;
	fd = new FeatureDetector(dhwi, window->getRenderer());
	// just comment this line if the GPR isnt plugged in
	//fd->initialise();
	Log::i << "-> FEATURE DETECTOR DONE" << endl << endl;
	
	Log::i << "-> Starting virtual platform display..." << endl;
	vp = new VirtualPlatform();
	vp->initialise(dhwi, ns, dc, window->getRenderer());
	Log::i << "-> VIRTUAL PLATFORM DONE" << endl << endl;
	


	initialised = true;

	Log::i << "--------------------------------------------------------------" << endl;

	return true;
}


void Overlord::run() {

	std::chrono::duration<double> seconds;
	std::chrono::duration<double> quadTimer;
	bool newPathDone = false;

	// metal detector dummy image
	SimpleTexture* mdtexture = new SimpleTexture(window->getRenderer());
	mdtexture->loadImage("metalDetectorOutput.png");
	
	// display the window for the first time
	window->showWindow(true);
	
	// Feature detector stuff
	fd->loadScan();
	fd->createImage(DISPLAY_KERNEL);
	//fd->runScan();
	//fd->createImage(DISPLAY_RAW);
	//window->update(fd->retrieveImage(), PANE_BOTTOMLEFT);
	window->update(NULL, PANE_BOTTOMRIGHT);

	showvp = true;
	vp->redrawGraphTexture();
	vp->drawPathToTexture();
	
	std::chrono::time_point<std::chrono::high_resolution_clock> t1;
	std::chrono::time_point<std::chrono::high_resolution_clock> t2;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();

	bool hasDone = false;

	while (!window->shouldQuit()) {

		// event handling in here. this stuff runs continuously ---------------//
		window->handleEvents();
		this->handleEvents();


		
		// periodic refresh shit. runs at 60Hz ish. ---------------------------//
		Log::setVerbosity(LOG_INFORMATIVE);

		current = std::chrono::high_resolution_clock::now();
		seconds = current - lastDataUpdate;
		quadTimer = current - startTime;

		if (quadTimer.count() > 5 && newPathDone == false) {
			ns->renewPath();
			ns->clearPath();
			ns->clearSubdividedPath();
			ns->addPoint(Point(0, 0));
			ns->addPoint(Point(20, 0));
			ns->addPoint(Point(20, 20));
			ns->addPoint(Point(24, 24));
			ns->subdivide(dhwi->getRealPosition(), dhwi->getRealAbsoluteHeading());
			vp->drawPathToTexture();
			ns->startPath();
			newPathDone = true;
		}
		
		/***********************************
		send dummy hardware stuff to quadbike
		************************************/
		hwi->setDesiredGear(dhwi->getRealGear());
		hwi->setDesiredThrottlePercentage(dhwi->getRealThrottlePercentage());
		hwi->setDesiredSteeringAngle(dhwi->getRealSteeringAngle());
		hwi->setDesiredBrakePercentage(dhwi->getRealBrakePercentage());
		/***********************************
		end of testing
		************************************/
		
		//Log::i << hwi->getPosition().x << ", " << hwi->getPosition().y << ", " << hwi->getAbsoluteHeading() << endl;
		//Log::i << hwi->getVelocity() << endl;

		if (seconds.count() > (1.0 / (double)DATA_REFRESH_RATE)) {
			lastDataUpdate = current;

			t1 = std::chrono::high_resolution_clock::now();
			vp->update();										// update logic
			t2 = std::chrono::high_resolution_clock::now();
			seconds = t2 - t1;
			//Log::i << "VP udpate: " << seconds.count()*1000 << " ms. Freq " << 1.0 / seconds.count() << endl;
			
		}

		seconds = current - lastWindowUpdate;

		if (seconds.count() > (1.0/(double)VIEW_REFRESH_RATE)) {
			lastWindowUpdate = current;

			t1 = std::chrono::high_resolution_clock::now();
			vp->redrawSimulationTexture();						// render to texture
			window->update(vp->retrieveSimulationImage(), PANE_TOPLEFT);	// render texture to window
			t2 = std::chrono::high_resolution_clock::now();
			seconds = t2 - t1;
			//Log::i << "Redraw udpate: " << seconds.count()*1000 << " ms. Freq " << 1.0 / seconds.count() << endl;

			t1 = std::chrono::high_resolution_clock::now();
			vp->redrawGraphTexture();
			window->update(vp->retrieveGraphImage(), PANE_TOPRIGHT);
			t2 = std::chrono::high_resolution_clock::now();
			seconds = t2 - t1;
			//Log::i << "Graphs udpate: " << seconds.count() * 1000 << " ms. Freq " << 1.0 / seconds.count() << endl;

			// this is STATIC ATM
			
			//fd->createImage(DISPLAY_RAW);
			window->update(fd->retrieveImage(), PANE_BOTTOMLEFT);
			window->update(mdtexture->getTexture(), PANE_BOTTOMRIGHT);


			window->present();

			dc->setEnabled(true);


		}

		Log::setVerbosity(LOG_ALL);
		// ---------------------------------------------------------------------//
	}
}



void Overlord::onEvent(Packet* packet) {
	packets.push(packet);
}


void Overlord::handleEvents() {

	while (packets.size() > 0) {
		// do stuff with the packets
		Packet* p = packets.front();
		bool handled = false;
		//Log::d << "Processing packet..." << endl;

		switch (p->packetID) {
		case ID_EMERGENCY_STOP:
			Log::e << "Action: EMERGENCY STOP" << endl;
			hwi->emergencyStop();
			handled = true;
			break;
		case ID_DEBUG:
			Log::d << "Debug packet received" << endl;
			hwi->setDesiredGear(GEAR_FORWARD);
			handled = true;
			break;
		case ID_CLEAR_NAV_POINTS:
			Log::d << "Action: clear navigation points" << endl;
			handled = true;
			break;
		case ID_SHOW_FD:
			Log::d << "Action: show feature detector" << endl;
			showvp = false;
			handled = true;
			break;
		case ID_SHOW_VP:
			Log::d << "Action: show virtual platform" << endl;
			showvp = true;
			handled = true;
			break;
		case ID_STOP_ENGINE:
			Log::d << "Action: stop engine" << endl;
			handled = true;
			break;
		case ID_MANUALJOYSTICK: {
			//Log::d << "Joystick: " << p->data[0] << "degrees, magnitude " << p->data[1] << endl;
			dc->enableManualControl();
			float ang = p->data[0];	// in degrees
			float mag = p->data[1];
			float steerAng = ang - 90;
			if (steerAng < -180) steerAng += 360;
			if (steerAng > 180) steerAng -= 180;
			steerAng /= 3.75;
			if (steerAng > 24) {
				steerAng = -steerAng + 48;
			}
			if (steerAng < -24) {
				steerAng = -steerAng - 48;
			}
			float velocity = (float) (sin(ang * PI/180.0) * mag) * 1.2f;
			float steering = (float) (steerAng * PI/180);

			Log::d << "Steering: " << steering * 180/PI << " / Velocity: " << velocity << endl;
			dhwi->setDesiredVelocity(velocity);
			dhwi->setDesiredSteeringAngle(steering);

			handled = true;
			break;
		}
		case ID_JOYSTICK_HELD:
			Log::d << "Action: joystick enabled" << endl;
			dc->enableManualControl();
			handled = true;
			break;
		case ID_JOYSTICK_RELEASED:
			Log::d << "Action: joystick disabled" << endl;
			dc->disableManualControl();
			dhwi->setDesiredVelocity(0);
			dhwi->setDesiredSteeringAngle(0);
			handled = true;
			break;
		case ID_MANUALCONTROL_ON:
			Log::d << "Action: manual control enabled" << endl;
			handled = true;
			break;
		case ID_BRAKE:
			Log::d << "Action: brake" << endl;
			//TODO(Jono) this needs to not be here
			//dc->setEnabled(!dc->isEnabled());
			handled = true;
			break;
		case ID_HANDBRAKE_OFF:
			Log::d << "Action: handbrake off" << endl;
			handled = true;
			break;
		case ID_HANDBRAKE_ON:
			Log::d << "Action: handbrake on" << endl;
			handled = true;
			break;
		case ID_REQ_QUAD_SPEED:
		{
			float speed = (float)hwi->getVelocity();
			//Log::d << "Request: quad speed " << speed << endl;
			Packet* op = new Packet();
			op->packetID = ID_QUAD_SPEED;
			op->length = 1;
			op->data = new float[1];
			op->data[0] = speed;
			comms->send(op);
			handled = true;
			break;
		}
		case ID_REQ_QUAD_HEADING:
		{
			float head = (float) hwi->getAbsoluteHeading();
			//Log::d << "Request: quad speed " << speed << endl;
			Packet* op = new Packet();
			op->packetID = ID_QUAD_HEADING;
			op->length = 1;
			op->data = new float[1];
			op->data[0] = head;
			comms->send(op);
			handled = true;
			break;
		}
		case ID_REQ_QUAD_POSITION:
		{
			Point pos = hwi->getPosition();
			//Log::d << "Request: quad speed " << speed << endl;
			Packet* op = new Packet();
			op->packetID = ID_QUAD_POSITION;
			op->length = 2;
			op->data = new float[2];
			op->data[0] = (float)pos.x;
			op->data[1] = (float)pos.y;
			comms->send(op);
			handled = true;
			break;
		}
		case ID_NAV_PATH:
		{
			Log::i << "Received navigation path" << endl;
			dc->setEnabled(false);
			ns->clearPath();


			union u_tag {
				float f[2];
				double dval;
			} u;
			for (int i = 0; i < p->length; i += 4) {
				u.f[0] = p->data[i + 0];
				u.f[1] = p->data[i + 1];

				double lat = u.dval;

				u.f[0] = p->data[i + 2];
				u.f[1] = p->data[i + 3];

				double lon = u.dval;

				ns->addPoint(LatLng(lat, lon));
				

				Log::i << "\t Lat/Lng: " << std::setprecision(16) << lat << "E " << lon << "N " << endl;
				//Log::i << "\t Lat/Lng: " << std::setprecision(10) << p->data[i] << "E " << p->data[i + 1] << "N" << endl;
			}
			ns->subdivide(dhwi->getPosition(), (float)dhwi->getAbsoluteHeading());
			//vp->drawPathToTexture();
			handled = true;
			break;
		}
		case ID_NAV_ZONE:
		{
			Log::i << "Received navigation zone" << endl;
			
			union u_tag {
				float f[2];
				double dval;
			} u;
			for (int i = 0; i < p->length; i += 4) {
				u.f[0] = p->data[i+0];
				u.f[1] = p->data[i+1];

				double lat = u.dval;

				u.f[0] = p->data[i + 2];
				u.f[1] = p->data[i + 3];

				double lon = u.dval;
				Log::i << "\t Lat/Lng: " << std::setprecision(16) << lat << "E " << lon << "N " << endl;
				//Log::i << "\t Lat/Lng: " << std::setprecision(10) << p->data[i] << "E " << p->data[i + 1] << "N" << endl;
			}
			
			handled = true;
			break;
		}
		default:
			Log::e << "Packet Error: Unrecognised command received. [" << p->packetID << "]" << endl;
			handled = true;
		}

		if (handled) {
			packets.pop();
		}
	}
}











/*
Log::e << "PACKET" << endl;
Packet* p = new Packet();
p->packetID = ID_DEBUG;
p->length = 4;
p->data = new float[p->length];
p->data[0] = 6.4096906e-10f;
p->data[1] = 2.9764932e29f;
p->data[2] = -1.180104e-38f;
p->data[3] = -2.7211347e-19f;

comms->send(p);
*/