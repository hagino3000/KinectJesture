#include "handJesture.h"
#include "zhelpers.hpp"

//the ZMQ socket used to transfer data to the browser
zmq::context_t context (1);
zmq::socket_t socket (context, ZMQ_PUB);

//--------------------------------------------------------------
void HandJesture::setup() {
	ofSetLogLevel(0);
	ofLog(OF_LOG_VERBOSE, "Start setup()");
	
	debug = true;
	showConfigUI = true;
	mirror = true;
	
	// Setup Kinect
	angle = -5;
	//kinect.init(true);  //shows infrared image
	kinect.init();
	//kinect.setVerbose(true);
	kinect.open();
	kinect.setCameraTiltAngle(angle);
	
	// Setup ofScreen
	//ofSetFullscreen(true);
	ofSetFrameRate(30);
	ofBackground(0, 0, 0);
	ofSetWindowShape(800, 600);
	
	// For images
	grayImage.allocate(kinect.width, kinect.height);
	checkGrayImage.allocate(kinect.width, kinect.height);
	grayThresh.allocate(kinect.width, kinect.height);
	
	// For hand detection
	nearThreshold = 5;
	farThreshold = 30;
	detectCount = 0;
	detectTwoHandsCount = 0;
	
	displayWidth = 1280;
	displayHeight = 800;
	
	// Fonts
	msgFont.loadFont("Courier New.ttf",14, true, true);
	msgFont.setLineHeight(20.0f);
	
	/*
	try {
		socket.bind ("tcp://*:14444");
		s_sendmore (socket, "event");
        s_send (socket, "{type:\"up\"}");
		std::cout << "Open Zeromq socket" << endl;
	}
	catch (zmq::error_t e) {
		std::cerr << "Cannot bind to socket: " <<e.what() << endl;
		exit();
	}
	*/
	
	// Sounds
	soundDetect.loadSound("sound/16582__tedthetrumpet__kettleswitch1.aif");
	soundDetect.setVolume(100);
	soundRelease.loadSound("sound/2674__dmooney__TAPE32.wav");
	soundRelease.setVolume(100);
	
	// setup config gui
	gui.setup();
	gui.config->gridSize.x = 300;
	gui.addTitle("KINECT SETTINGS");
	gui.addSlider("Tilt Angle", angle, -30, 30);
	gui.addToggle("Mirror Mode", mirror);
	gui.addTitle("DETECT RANGE");
	gui.addSlider("Near Distance", nearThreshold, 5, 20);
	gui.addSlider("Far Distance", farThreshold, 20, 60);
	gui.addTitle("MOUSE CONTROLL");
	gui.addSlider("Display Width", displayWidth, 600, 1980);
	gui.addSlider("Display height", displayHeight, 600, 1980);
	gui.setDefaultKeys(true);
	gui.loadFromXML();
	gui.show();

}

//--------------------------------------------------------------
void HandJesture::update() {
		
	kinect.update();
	checkDepthUpdated();
	
	grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
	grayThresh.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
	
	unsigned char * pix = grayImage.getPixels();
	unsigned char * grayThreshPix = grayThresh.getPixels();
	int numPixels = grayImage.getWidth() * grayImage.getHeight();

	int maxThreshold = 255 - nearThreshold;// default 250
	int minThreshold = 255 - farThreshold; // default 225
	
	int nearestDepth = 0;
	for(int i = 0; i < numPixels; i++){
		if (minThreshold < pix[i] && pix[i] < maxThreshold && pix[i] > nearestDepth) {
			nearestDepth = pix[i];
		}
	}
		
	for(int i = 0; i < numPixels; i++){
		//if( pix[i] < nearThreshold && pix[i] > farThreshold ){
		if(minThreshold < pix[i] 
	    && pix[i] < nearestDepth+2 
		&& pix[i] > nearestDepth - 10 ){
			//grayThreshPix[i] = floor((5 - (nearestDepth - pix[i]))/5.0f*255.0f);
			pix[i] = 255; // white
		}else{
			pix[i] = 0;
			//grayThreshPix[i] = 0;
		}
	}
	
	//update the cv image
	grayImage.flagImageChanged();
	//grayThresh.flagImageChanged();
	if (mirror) {
		grayImage.mirror(false, true);
	}
	//grayThresh.mirror(false, true);
	
    contourFinder.findContours(grayImage, 1500, (kinect.width*kinect.height)/4, 2, false);
	
	if (showConfigUI) {
		return;
	}
	
	int detectHands = contourFinder.nBlobs;
	
	if (detectHands == 2) {
		detectTwoHandsCount = min(60, ++detectTwoHandsCount);
	} else {
		detectTwoHandsCount = max(0, --detectTwoHandsCount);
	}
	
	if (detectHands > 0) {
		detectCount = min(50, ++detectCount);
	} else {
		detectCount = max(0, --detectCount);
	}
	
	if (detectingHands) {
		if (detectCount < 10) {
			detectingHands = false;
			sendEvent("UnRegister", "\"mode\":\"single\"");
			for (int j = 0; j < hands.size(); j++) {
				hands[j]->unRegister();
			}
			soundRelease.play();
		}
	} else {
		if (detectCount > 30) {
			detectingHands = true;
			sendEvent("Register", "\"mode\":\"single\"");
			soundDetect.play();
		}
	}

	ofLog(OF_LOG_VERBOSE, ofToString(detectTwoHandsCount));
	if (detectingTwoHands) {
		ofLog(OF_LOG_VERBOSE, "detecTwo");
		if (detectTwoHandsCount < 15) {
			detectingTwoHands = false;
			sendEvent("Register", "\"mode\":\"double\"");
		}
	} else {
		ofLog(OF_LOG_VERBOSE, "Not...");
		if (detectTwoHandsCount > 30) {
			detectingTwoHands = true;
			sendEvent("Register", "\"mode\":\"double\"");
			CGEventRef keyEv = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)101, true);
			CGEventPost (kCGHIDEventTap, keyEv);
			CGEventRef keyEv2 = CGEventCreateKeyboardEvent(NULL, (CGKeyCode)101, false);
			CGEventPost (kCGHIDEventTap, keyEv2);

		}
	}
	
	if (detectingHands && detectHands == 1) {
				
		for (int i = 0; i < contourFinder.nBlobs; i++){
			int centerX = contourFinder.blobs[i].centroid.x;
			int centerY = contourFinder.blobs[i].centroid.y;

			// Apply lowpass filter
			float x = centerX;
			float y = centerY;
			int cornerCount = contourFinder.blobs[i].nPts;
			
			
			float centroidX = 0;
			float centroidY = 0;
			float addCount = 0;
			for (int j = 0; j < contourFinder.blobs[i].nPts; j+=5){
				addCount++;
				centroidX += contourFinder.blobs[i].pts[j].x;
				centroidY += contourFinder.blobs[i].pts[j].y;
			}
			centroidX = centroidX/addCount;
			centroidY = centroidY/addCount;			
			
			if (hands.size() == 0) {
				Hand *hand = new Hand(true, displayWidth, displayHeight);
				hand->setIsActive(true);
				hand->update(ofPoint(x, y), cornerCount, ofPoint(x, y));
				hands.push_back(hand);
			} else {
				for (int j = 0; j < hands.size(); j++) {
					hands[j]->update(ofPoint(x, y), cornerCount, ofPoint(centroidX, centroidY));
				}
			}

			/*
			stringstream ss;
			ss  << "\"x\":"  << x
            << ",\"y\":" << y
            << ",\"z\":" << min(100, (int)kinect.getDistanceAt(centerX, centerY));
			//cout << "move: " << ss.str() << endl;
			sendEvent("Move", ss.str());
			*/
		}
	}
	
	if (detectingTwoHands) {
		// scroll
	}
}

void HandJesture::sendEvent(const std::string& etype, const std::string& edata) {
	s_sendmore (socket, "event");
	stringstream ss;
	ss << "{\"type\":\"" << etype << "\",\"data\":{" << edata << "}}";
	s_send (socket, ss.str());
	//std::cout << ss.str() << endl;
}


/**
 * Check depth data is updated.
 *
 * If not updated, close and reopen the Kinect.
 */
void HandJesture::checkDepthUpdated(){
    if (ofGetFrameNum() % 150 == 0) {
		ofLog(OF_LOG_VERBOSE, "check depth updated");
        unsigned char * nextDepth = kinect.getDepthPixels();
		
        if (ofGetFrameNum() != 150) {
			ofLog(OF_LOG_VERBOSE, "Start compare depth pixels");
			unsigned char * currentDepthPixels = checkGrayImage.getPixels();
			
		    int pixNum = kinect.width * kinect.height;
            for (int i=0; i<pixNum; i++) {
                if (nextDepth[i] != currentDepthPixels[i]) {
                    break;
				}
				if (i > pixNum / 2) {
					ofLog(OF_LOG_ERROR, "Depth pixels could not be refreshed. Reset Kinect");
					kinect.close();
					kinect.open();
					kinect.setCameraTiltAngle(angle);
					break;
				}
			}                  
		}
		checkGrayImage.setFromPixels(nextDepth, kinect.width, kinect.height);
	}
}



//--------------------------------------------------------------
void HandJesture::draw() {
	
	ofSetColor(255, 255, 255);
	
	if (showConfigUI) {
		kinect.drawDepth(400, 0, 400, 300);
		gui.draw();
		
		msgFont.drawString("Press Space Key to start.", 20, ofGetHeight()-60);
		
		ofPushMatrix();
		ofTranslate(400, 300, 0);
		glScalef(0.6, 0.6, 1.0f); 
        for (int i = 0; i < contourFinder.nBlobs; i++){
            ofPushMatrix();
            contourFinder.blobs[i].draw(0,0);
			ofSetColor(255, 0, 0);
            ofFill();
            ofEllipse(contourFinder.blobs[i].centroid.x, contourFinder.blobs[i].centroid.y, 4, 4);
			
			float centroidX = 0;
			float centroidY = 0;
			float addCount = 0;
			for (int j = 0; j < contourFinder.blobs[i].nPts; j+=5){
				addCount++;
				centroidX += contourFinder.blobs[i].pts[j].x;
				centroidY += contourFinder.blobs[i].pts[j].y;
			}
			centroidX = centroidX/addCount;
			centroidY = centroidY/addCount;
			ofCircle(centroidX, centroidY, 10);
			
            ofPopMatrix();
        }
		ofPopMatrix();
	} else {
		grayImage.draw(0, 0, 400, 300);
	}
	
		
	ofSetColor(255, 255, 255);
	msgFont.drawString("fps: "+ ofToString(ofGetFrameRate()), 20, ofGetHeight()-40);
		
	ofNoFill();
}

//--------------------------------------------------------------
void HandJesture::exit(){
	kinect.close();
	ofLog(OF_LOG_NOTICE, "Close Kinect and exit");
}


//--------------------------------------------------------------
void HandJesture::keyPressed (int key)
{
	ofLog(OF_LOG_VERBOSE, ofToString(key));
	switch (key)
	{
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
		case '<':		
		case ',':		
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
		case '-':		
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
        case 'd':
            //解析結果の表示の on / off
            debug ? debug=false : debug=true;
            break;
		case 'r':
			// reboot kinect
			kinect.close();
			kinect.open();
			kinect.setCameraTiltAngle(angle);
            break;
		case ' ':
			showConfigUI = !showConfigUI;
			if (showConfigUI) {
				ofSetWindowShape(800, 600);
			} else {
				ofSetWindowShape(400, 300);
				kinect.setCameraTiltAngle(angle);
			}
			break;			
		case OF_KEY_UP:
			angle++;
			if(angle>30) angle=30;
			kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
			angle--;
			if(angle<-30) angle=-30;
			kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void HandJesture::mouseMoved(int x, int y)
{}

//--------------------------------------------------------------
void HandJesture::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void HandJesture::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void HandJesture::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void HandJesture::windowResized(int w, int h)
{}

