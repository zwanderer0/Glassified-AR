#pragma once

#include "ofMain.h"

#include "ofxOpenCv.h"

//#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
								// otherwise, we'll use a movie file

class testApp {

	public:
		testApp();
		void Main();
		//int cal(char *e, int length)
		float cal(float *e, int length);
		int parseInt(float *e, float &a, int length);

};

