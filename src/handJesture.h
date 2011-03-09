#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxSimpleGuiToo.h"
#include "hand.h"

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <vector>


class HandJesture : public ofBaseApp
{
	
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	
	void keyPressed  (int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	void drawPointCloud();
	void drawHands();
	void drawHandCircle();
	void drawBackground();
	void sendEvent(const std::string& event, const std::string& data);
	void detectCorner();
	void checkDepthUpdated();
	
	// Kinect
	ofxKinect kinect;
	int angle;
	
	ofxCvColorImage		colorImage;
	ofxCvGrayscaleImage		checkGrayImage;
	
	ofxCvGrayscaleImage 	grayImage;
	ofxCvGrayscaleImage 	grayThresh;
	ofxCvGrayscaleImage		grayThreshPrev;
	
	ofxCvContourFinder 	contourFinder;
	
	bool				debug;
	bool				showConfigUI;
	bool				mirror;
	
	// for image
	int 				nearThreshold;
	int					farThreshold;
	
	// for mouse controll
	int					displayWidth;
	int					displayHeight;
	
	// sounds
	ofSoundPlayer		soundDetect;
	ofSoundPlayer		soundRelease;

	
	// for detection
	int					detectCount;
	int					detectTwoHandsCount;
	bool				detectingHands;
	bool				detectingTwoHands;
	vector<Hand *>		hands;
	
	
	// fonts
	ofTrueTypeFont		msgFont;
};

#endif
