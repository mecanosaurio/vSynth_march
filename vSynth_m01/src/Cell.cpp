#include "Cell.h"


Cell::Cell() {}
Cell::~Cell(){}


void Cell::setup(ofPoint position, int ntr) {
	pos = position;
	dest = ofPoint(ofGetWidth()/2, pos.y, pos.z);
	spc_trail = 0;
	trail.resize(0);
	nt = ntr;
	for (int i = 0; i < nt; i++) {
		trail.push_back(ofPoint(pos.x + i*spc_trail, pos.y, pos.z));
	}
	n = int(ofRandom(3, 8));
	r = ofRandom(20, 50);
	v = ofRandom(0.01, 0.5);
	cW = 1;
	cH = 20;
	cwd = 10;
	//if (int(r) % 2 == 0) { v = -v; }
	gravity = ofMap(r, 20, 50, 5, 30);
	color = ofColor(0,255,0, 172);
	shape.clear();
	isDead = false;
	isExpl = false;
}


void Cell::fall() {
	pos.y += gravity;
	if (pos.y > ofGetHeight()) {
		isDead = true;
	}
};


void Cell::reset() {
	n = int(ofRandom(3, 8));
	r = ofRandom(20, 50);
	v = ofRandom(3, 7);
	if (int(r) % 2 == 0) { v = -v; }
	gravity = ofMap(r, 20, 50, 3, 20);
}


void Cell::go_to() {
	pos.x = ofLerp(pos.x, dest.x, 0.04);
	for (int i = 0; i < trail.size(); i++) {
		trail[i].x = pos.x + i*spc_trail;
	}
	cW = ofLerp(cW, cwd, 0.1);
	spc_trail = ofLerp(spc_trail, spc_td, 0.1);
	// --
	pos.x += v;
	dest.x += v;
	if ( pos.x >= ofGetWidth() ) {
		pos.x -= (ofGetWidth() + cW);
		dest.x = pos.x;
	}
};

void Cell::update(float tt) {
	pos.x = ofGetWidth()/2 + ofMap(ofNoise(tt), 0, 1, -ofGetWidth() / 2, ofGetWidth() / 2);
}

void Cell::draw() {
	color.setHue(int(ofGetFrameNum()/10) % 255);
	ofSetColor(color, 132);
	//ofSetLineWidth(4);
	//shape.draw();
	//ofSetColor(ofColor(255,0,0), 205);
	for (int i = 0; i < trail.size(); i++){
		ofDrawRectangle(trail[i].x, pos.y, 1 + cW, cH);
		if (trail[i].x + cW > ofGetWidth()) {
			ofDrawRectangle(trail[i].x - (ofGetWidth() + cW) , pos.y, 1 + cW, cH);
		}
	}
};


bool Cell::isInside(ofPoint p) {
	if (shape.inside(p)) {
		return true;
	} else {
		return false;
	}
};


void Cell::explode() {
	color = ofColor(255, ofMap(r, 10, 100, 255, 50));
	if (r < 100) {
		r+=5;
	} else { 
		isDead = true;
	}
}
	