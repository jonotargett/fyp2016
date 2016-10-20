/* Feature detector creates images from the relevant scans and produces correct windows*/
#include "FeatureDetector.h"
#include "Window.h"
#include "SimpleTexture.h"

FeatureDetector::FeatureDetector(HardwareInterface* interf, SDL_Renderer* r) : hwi(interf), renderer(r)
{
	generateColorMap();
}


FeatureDetector::~FeatureDetector()
{
	SDL_DestroyTexture(texture);
	texture = NULL;
}

bool FeatureDetector::initialise() {

	bool success;

	gpr = new GPR();
	md = new MD();

	success = gpr->initialise();
	if (!success) return false;


	scan = gpr->getBscan(GPR_DIFFERENTIAL);
	//scan = gpr->getBscan(GPR_CHANNEL_1);

	success = md->initialise();
	if (!success) return false;



	return true;
}

bool FeatureDetector::runScan() {
	bool received = false;

	//while (!received) {
	//	received = gpr->getData();
	//}

	return true;
}

bool FeatureDetector::loadScan() {

	delete scan;
	scan = new Bscan();
	Bscan* empty = new Bscan();

	/*
	FILENAMES:

	01011329.DAT	- empty
	01011330.DAT	- empty
	01011348.DAT	- empty
	01041445.DAT	- ST-AT(1)-flat-at0.5cm Pepsi-perp-at0.5cm
	01031453.DAT	- filledPepsi-perp-at0.5cm Pepsi-flat(upright)-at0.5cm
	01021000.DAT	- PVC10x15-para-at0.5cm SS10x05-at0.5cm
	*/

	//empty->load("./scans/01011329.DAT");
	//Log::i << "empty scan loaded." << std::endl;
	//scan->load("./scans/01031453.DAT");
	//scan->loadPlainText("./scans/All-1.4GHz-APC-Test3.dat");
	scan->loadRDR("./scans/All-1.4GHz-ATT-Test1.rdr", ANT_CHANNEL1);
	empty->loadRDR("./scans/All-1.4GHz-ATT-Test1.rdr", ANT_CHANNEL_DIFF);
	//scan->loadRDR("./scans/All-1.4GHz-APC-Test2.rdr", ANT_CHANNEL1);
	//empty->loadRDR("./scans/All-1.4GHz-APC-Test2.rdr", ANT_CHANNEL_DIFF);
	//scan->loadRDR("./scans/All-1.4GHz-APM-Test2.rdr", ANT_CHANNEL1);
	//empty->loadRDR("./scans/All-1.4GHz-APM-Test2.rdr", ANT_CHANNEL_DIFF);
	
	Log::i << "feature scan loaded." << std::endl;

	Ascan* normal = scan->produceNormal(scan->length());
	Log::i << "background noise identified from antenna differential." << std::endl;
	scan->normalise(normal);
	Log::i << "feature scan normalised for background noise." << std::endl;

	return true;
}

bool FeatureDetector::createImage(Visual displayMode) {

	SDL_DestroyTexture(texture);
	texture = NULL;

	
	image = SDL_CreateRGBSurface(0, 1000, 500, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	Uint32 *pixels = (Uint32*)image->pixels;

	// initialise the surface to blue
	for (int i = 0; i < image->w*image->h; i++) {
		pixels[i] = SDL_MapRGB(image->format, 0x00, 0x00, 0x00);
	}

	// if a scan is paired to this window, push it to the screen
	if (scan != NULL) {

		int cols = std::min(image->w, scan->length()-1);

		for (int i = 0; i < cols; i++) {
			Ascan* a = scan->element(i);
			int rows = std::min((unsigned int)image->h, a->getSize());


			for (int j = 0; j < rows; j++) {

				int v;
				if (displayMode == DISPLAY_RAW) {
					v = a->getIndex(j);
				}
				if (displayMode == DISPLAY_KERNEL) {
					v = scan->Kernel(3, i, j);
				}

				//int h = v / 256;
				//int l = v - h * 256;

				//int16_t n = (int16_t)(v);
				//uint8_t h = ((int)(v) / 256);
				//uint8_t l = ((int8_t)v);


				float gain = (float)(pow(((float)j*3.0 / (float)rows), 3.0) + 1.0f);
				v = (int) (v * gain);

				uint32_t offset = ((uint16_t)v) * 3;

				while (offset >= 65536 * 3) {
					offset -= 65536 * 3;
				}

				uint8_t r = colormap[offset + 0];
				uint8_t g = colormap[offset + 1];
				uint8_t b = colormap[offset + 2];

				//Log::e << (int)offset << "-" << endl;


				pixels[j*image->w + i] = SDL_MapRGB(image->format, r, g, b);
			}
		}


		Log::d << "Visual scan image created." << std::endl; 
		texture = SDL_CreateTextureFromSurface(renderer, image);

		if (texture == NULL) {
			Log::e << "Could not create texture from scan image. " << SDL_GetError() << endl;
		}
	}
	else {
		Log::d << "Scan is NULL" << endl;
	}

	SDL_FreeSurface(image);

	return true;
}


SDL_Texture* FeatureDetector::retrieveImage() {
	return texture;
}



void FeatureDetector::generateColorMap() {

	float initial_map[65*3] = {
		0,	0,	0.562500000000000,
		0,	0,	0.625000000000000,
		0,	0,	0.687500000000000,
		0,	0,	0.750000000000000,
		0,	0,	0.812500000000000,
		0,	0,	0.875000000000000,
		0,	0,	0.937500000000000,
		0,	0,	1,
		0,	0.062500000000000,	1,
		0,	0.125000000000000,	1,
		0,	0.187500000000000,	1,
		0,	0.250000000000000,	1,
		0,	0.312500000000000,	1,
		0,	0.375000000000000,	1,
		0,	0.437500000000000,	1,
		0,	0.500000000000000,	1,
		0,	0.562500000000000,	1,
		0,	0.625000000000000,	1,
		0,	0.687500000000000,	1,
		0,	0.750000000000000,	1,
		0,	0.812500000000000,	1,
		0,	0.875000000000000,	1,
		0,	0.937500000000000,	1,
		0,	1,	1,
		0.062500000000000,	1,	0.937500000000000,
		0.125000000000000,	1,	0.875000000000000,
		0.187500000000000,	1,	0.812500000000000,
		0.250000000000000,	1,	0.750000000000000,
		0.312500000000000,	1,	0.687500000000000,
		0.375000000000000,	1,	0.625000000000000,
		0.437500000000000,	1,	0.562500000000000,
		0.500000000000000,	1,	0.500000000000000,
		0.562500000000000,	1,	0.437500000000000,
		0.625000000000000,	1,	0.375000000000000,
		0.687500000000000,	1,	0.312500000000000,
		0.750000000000000,	1,	0.250000000000000,
		0.812500000000000,	1,	0.187500000000000,
		0.875000000000000,	1,	0.125000000000000,
		0.937500000000000,	1,	0.062500000000000,
		1,	1,	0,
		1,	0.937500000000000,	0,
		1,	0.875000000000000,	0,
		1,	0.812500000000000,	0,
		1,	0.750000000000000,	0,
		1,	0.687500000000000,	0,
		1,	0.625000000000000,	0,
		1,	0.562500000000000,	0,
		1,	0.500000000000000,	0,
		1,	0.437500000000000,	0,
		1,	0.375000000000000,	0,
		1,	0.312500000000000,	0,
		1,	0.250000000000000,	0,
		1,	0.187500000000000,	0,
		1,	0.125000000000000,	0,
		1,	0.062500000000000,	0,
		1,	0,	0,
		0.937500000000000,	0,	0,
		0.875000000000000,	0,	0,
		0.812500000000000,	0,	0,
		0.750000000000000,	0,	0,
		0.687500000000000,	0,	0,
		0.625000000000000,	0,	0,
		0.562500000000000,	0,	0,
		0.500000000000000,	0,	0,
		0.437500000000000,	0,	0
	};


	colormap = new uint8_t[65536 * 3];
	int offset = 0;
	float c1_r, c1_g, c1_b, c2_r, c2_g, c2_b;

	for (int i = 0; i < 64; i++) {

		c1_r = initial_map[(i + 0) * 3 + 0] * 256;
		c1_g = initial_map[(i + 0) * 3 + 1] * 256;
		c1_b = initial_map[(i + 0) * 3 + 2] * 256;
		c2_r = initial_map[(i + 1) * 3 + 0] * 256;
		c2_g = initial_map[(i + 1) * 3 + 1] * 256;
		c2_b = initial_map[(i + 1) * 3 + 2] * 256;

		/*
		Log::d << c1_r << "-" << c1_g << "-" << c1_b 
			<< "/" << c2_r << "-" << c2_g << "-" << c2_b << endl;
		*/

		for (int j = 0; j < 1024; j++) {

			float lerp = (float)j / 1024.0f;

			colormap[offset * 3 + 0] = (uint8_t)(clip(c1_r + (c2_r - c1_r)*(lerp), 0, 255));
			colormap[offset * 3 + 1] = (uint8_t)(clip(c1_g + (c2_g - c1_g)*(lerp), 0, 255));
			colormap[offset * 3 + 2] = (uint8_t)(clip(c1_b + (c2_b - c1_b)*(lerp), 0, 255));

			/*
			Log::i << lerp << "/" << (int)colormap[offset * 3 + 0] << "-"
				<< (int)colormap[offset * 3 + 1] << "-"
				<< (int)colormap[offset * 3 + 2] << endl;
			*/

			++offset;
		}

	}

	Log::d << "Colormap successfully built with " << offset << " entries" << endl;
}


float FeatureDetector::clip(float n, float lower, float upper) {
	return std::max(lower, std::min(n, upper));
}