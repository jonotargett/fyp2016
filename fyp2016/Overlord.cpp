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
	//hwi = new QuadInterface();
	hwi = new DummyHardware();
	hwi->initialise();
	Log::i << "-> HARDWARE INTERFACE DONE" << endl << endl;

	Log::i << "-> Initialising navigation system..." << endl;
	ns = new SimpleNavigator();
	ns->initialise();
	Log::i << "-> NAVIGATION SYSTEM DONE" << endl << endl;
	/*
	Log::i << "-> Initialising dummy hardware interface..." << endl;
	dhwi = new DummyHardware();
	dhwi->initialise();
	Log::i << "-> DUMMY HARDWARE INTERFACE DONE" << endl << endl;
	*/
	Log::i << "-> Initialising drive controller..." << endl;
	dc = new SimpleController();
	dc->initialise(hwi, ns);
	Log::i << "-> DRIVE CONTROLLER DONE" << endl << endl;

	Log::i << "-> Starting feature detection system..." << endl;
	fd = new FeatureDetector(hwi, window->getRenderer());
	// just comment this line if the GPR isnt plugged in
	//fd->initialise();
	Log::i << "-> FEATURE DETECTOR DONE" << endl << endl;
	
	Log::i << "-> Starting virtual platform display..." << endl;
	vp = new VirtualPlatform();
	vp->initialise(hwi, ns, dc, window->getRenderer());
	Log::i << "-> VIRTUAL PLATFORM DONE" << endl << endl;
	


	initialised = true;

	Log::i << "--------------------------------------------------------------" << endl;

	return true;
}


void Overlord::run() {

	std::chrono::duration<double> seconds;
	

	// Feature detector stuff
	fd->loadScan();
	fd->createImage(DISPLAY_KERNEL);

	// display the window for the first time
	window->showWindow(true);
	window->update(fd->retrieveImage(), PANE_BOTTOMLEFT);

	showvp = true;

	while (!window->shouldQuit()) {

		// event handling in here. this stuff runs continuously ---------------//
		
		window->handleEvents();
		this->handleEvents();
		

		// periodic refresh shit. runs at 60Hz ish. ---------------------------//
		Log::setVerbosity(LOG_INFORMATIVE);

		current = std::chrono::high_resolution_clock::now();
		seconds = current - lastWindowUpdate;

		if (seconds.count() > (1.0/(double)REFRESH_RATE)) {
			lastWindowUpdate = current;
			
			//window->clearWindow(PANE_ALL);

				vp->update();										// update logic
				
				vp->redrawSimulationTexture();						// render to texture
				window->update(vp->retrieveSimulationImage(), PANE_TOPLEFT);	// render texture to window
				vp->redrawGraphTexture();
				window->update(vp->retrieveGraphImage(), PANE_TOPRIGHT);

				window->update(NULL, PANE_BOTTOMRIGHT);

				// this is STATIC ATM
				//window->update(fd->retrieveImage(), PANE_BOTTOMRIGHT);


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
			handled = true;
			break;
		case ID_DEBUG:
			Log::d << "Debug packet received" << endl;
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
		case ID_MANUALJOYSTICK:
			//Log::d << "Joystick: " << p->data[0] << "degrees, magnitude " << p->data[1] << endl;
			handled = true;
			break;
		case ID_JOYSTICK_HELD:
			Log::d << "Action: joystick enabled" << endl;
			handled = true;
			break;
		case ID_JOYSTICK_RELEASED:
			Log::d << "Action: joystick disabled" << endl;
			handled = true;
			break;
		case ID_MANUALCONTROL_ON:
			Log::d << "Action: manual control enabled" << endl;
			handled = true;
			break;
		case ID_BRAKE:
			Log::d << "Action: brake" << endl;
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
			float speed = hwi->getVelocity();
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
			float head = hwi->getAbsoluteHeading();
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
			op->data[0] = pos.x;
			op->data[1] = pos.y;
			comms->send(op);
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