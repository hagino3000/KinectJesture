/*
 *  hand.cpp
 *  jestureCap
 *
 *  Created by Takashi on 11/02/12.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "hand.h"

int mouseDownCount = 0;
int MOUSE_CLICK_FRAME = 15;
int HAND_MODE_NORMAL = 0;
int HAND_MODE_MOVE = 1;
int HAND_MODE_CLICK = 2;
int HAND_MODE_DRAG = 3;

//コンストラクタ
Hand::Hand(bool isPrimary, int dispWidth, int dispHeight) {
	
	isPrimary = isPrimary;
	isActive = false;
	isMouseDown = false;
	isMouseDrag = false;
	displayHeight = dispHeight;
	displayWidth = dispWidth;
	mouseDownCount = 0;
	handMode = HAND_MODE_NORMAL; // NORMAL
	//centroid = ofPoint(0, 0);
	
	mouseDownCount = 0;
}

Hand::~Hand(){}

void Hand::update(ofPoint pos, int cornerCount, ofPoint currentCentroid) {
		
	currentTmpPos = getCurrentPos(pos);
	//ofLog(OF_LOG_VERBOSE, "HAND_MODE" + ofToString(handMode) + " : " + ofToString(currentTmpPos.x));
	
	if (handMode == HAND_MODE_NORMAL) {
		if (!checkClick(cornerCount)) {
			checkSpeedMove();
			setPos(currentTmpPos);
			fireMouseMove();
		}
	}
	if (handMode == HAND_MODE_MOVE) {
		checkSpeedMove();
		setPos(currentTmpPos);
		fireMouseMove();		
	}
	if (handMode == HAND_MODE_DRAG) {
		if (!checkClick(cornerCount)) {
			setPos(currentTmpPos);
			fireMouseDrag();
		}		
	}
	if (handMode == HAND_MODE_CLICK) {
		checkClick(cornerCount);
	}
}

void Hand::unRegister() {
	if (isMouseDrag) {
		fireMouseUp();
	}
	isMouseDrag = false;
	isMouseDown = false;
	isActive = false;
	handMode = HAND_MODE_NORMAL;
}

CGPoint Hand::calcMousePosition() {
	float x = currentPos.x;
	float y = currentPos.y;
		
	x = max(0.0, x*100.0/640.0-10.0f);
	x = min(x, 80.0f)/80*100;
	y = max(0.0, y*100.0/480.0-10.0f);
	y = min(y, 80.0f)/80*100;
	
	CGPoint pt;
	pt.x = x/100*displayWidth;
	pt.y = y/100*displayHeight;
	
	return pt;
}
bool Hand::checkSpeedMove() {
	return false;
}

bool Hand::checkClick(int cornerCount) {
	cornerCountHistory.push_back(cornerCount);
	if (cornerCountHistory.size() > 6) {
		cornerCountHistory.erase(cornerCountHistory.begin());
	} else {
		return false;
	}
		
	int oldCornerNums = 0;
	int cornerNums = 0;
	for (int i=0; i<cornerCountHistory.size(); i++) {
		if (i < 4) {
			oldCornerNums += cornerCountHistory[i];
		} else {
			cornerNums += cornerCountHistory[i];
		}
	}
	oldCornerNums = oldCornerNums/4;
	cornerNums = cornerNums/2;
	
	//cout << ofToString(abs(oldCornerNums - cornerNums)) << endl;
	
	bool isClicking = false;
	if (abs(oldCornerNums - cornerNums) > 100) {
		isClicking = true;
	}
	
	
	if (handMode == HAND_MODE_NORMAL && cornerNums + 160 < oldCornerNums) {
		// mouse down
		currentCornerNums = cornerNums;
		//fireMouseDown();
		//clikcedPos = calcMousePosition();
		//isMouseDown = true;
		handMode = HAND_MODE_CLICK;
		mouseDownCount = 0;
		return true;
	}
	if (cornerNums > currentCornerNums + 160) {
		if (handMode == HAND_MODE_DRAG) {
			fireMouseUp();
			handMode = HAND_MODE_NORMAL;
			return true;
		} else if (handMode == HAND_MODE_CLICK) {
			fireMouseClick();
			handMode = HAND_MODE_NORMAL;
			return true;
		}
	}
	if (handMode == HAND_MODE_CLICK) {
		mouseDownCount++;
		if (mouseDownCount > MOUSE_CLICK_FRAME) {
			handMode = HAND_MODE_DRAG;
			fireMouseDown();
			mouseDownCount = 0;
		}
	}
	return false;
}

void Hand::fireMouseMove() {
	CGPoint pt = calcMousePosition();
		
	// Mouse move
	CGEventRef mouseMoveEv = CGEventCreateMouseEvent (NULL, kCGEventMouseMoved, pt, kCGMouseButtonLeft);
	CGEventPost (kCGHIDEventTap, mouseMoveEv);
}

void Hand::fireMouseDown() {
	cout << "Mouse Down!!+++++++" << endl;
	CGPoint pt = calcMousePosition();
	CGEventRef mouseDownEv = CGEventCreateMouseEvent (NULL,kCGEventLeftMouseDown,pt,kCGMouseButtonLeft);
	CGEventPost (kCGHIDEventTap, mouseDownEv);
}

void Hand::fireMouseUp() {
	cout << "Mouse Up!!+++++++-----------" << endl;
	CGPoint pt = calcMousePosition();
	CGEventRef mouseUpEv = CGEventCreateMouseEvent (NULL,kCGEventLeftMouseUp,pt,kCGMouseButtonLeft);
	CGEventPost (kCGHIDEventTap, mouseUpEv );				
}

void Hand::fireMouseDrag() {
	CGPoint pt = calcMousePosition();
	CGEventRef mouseDragEv = CGEventCreateMouseEvent (NULL,kCGEventLeftMouseDragged,pt,kCGMouseButtonLeft);
	CGEventPost (kCGHIDEventTap, mouseDragEv );					
}

void Hand::fireMouseClick() {
	fireMouseDown();
	fireMouseUp();
}

ofPoint Hand::getCurrentPos(ofPoint newPos) {
	
	if (posHistory.size() == 0) {
		return newPos;
	}
	
	float x = newPos.x;
	float y = newPos.y;
	
	for (int j = 0; j < posHistory.size(); j++) {
		x += posHistory.at(j).x;
		y += posHistory.at(j).y;
	}
	
	x = x/(posHistory.size()+1);
	y = y/(posHistory.size()+1);
	
	return ofPoint(x, y);	
}

void Hand::setPos(ofPoint pos) {
	currentPos = pos;
	
	posHistory.push_back(pos);
	if (posHistory.size() > 4) {
		posHistory.erase(posHistory.begin());
	}	
}

void Hand::setIsActive(bool active) {
	isActive = active;
}

ofPoint Hand::getPos() {
	return currentPos;
}

bool Hand::getIsPrimary() {
	return isPrimary;
}

