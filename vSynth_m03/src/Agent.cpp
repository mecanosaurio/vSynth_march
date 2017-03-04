#include "Agent.h"


Agent::Agent() {}
Agent::~Agent(){}


void Agent::setup(ofPoint position) {
	pos = position;
	n = int(ofRandom(3, 8));
	r = 100;
	v = ofRandom(0.01, 0.5);
	cW = 1;
	cH = 20;
	dwell = ofRandom(0, 100);

	// create pattern
	nt = 222;
	trail.resize(0);
	ofPoint tPo = ofPoint(0, 0, 0);
	for (int i = 0; i < nt; i++) {
		tPo.x = r * cos(i*TWO_PI / nt);
		tPo.y = r * sin(i*TWO_PI / nt);
		trail.push_back(tPo);
	}

	//color = ofColor(ofRandom(155, 255), ofRandom(155, 255), ofRandom(155, 255), 92);
	color = ofColor(ofRandom(205, 255), ofRandom(2, 255), ofRandom(12, 255));
	shape.clear();
	//tess = ofTessellator();
	isDown = false;
	isDead = false;
	isExpl = false;
}


void Agent::fall(float g, float a) {
	gravity = g;
	atract = a;
	pos.y += gravity;
	if (pos.y > ofGetHeight()+r) pos.y = -r;
	if (pos.y < (-r) ) pos.y = ofGetHeight() + r;
	
	pos.x += a;
	if (pos.x > ofGetWidth()+r) pos.x = -r;
	if (pos.x < (-r) ) pos.x = ofGetWidth()+r;

	pos.z -= dwell;
	if (pos.z < -500) {
		pos.z = -500;
		isDown = true;
	}
};

void Agent::grow(float rate) {
	r += rate;
	color = ofColor(color, 92 - r*92/1000);
	if (r > 1000) isDead = true;
}


void Agent::reset() {
	n = int(ofRandom(3, 8));
	r = ofRandom(20, 50);
	v = ofRandom(3, 7);
	if (int(r) % 2 == 0) { v = -v; }
	gravity = ofMap(r, 20, 50, 3, 20);
}


void Agent::go_to() {
};

void Agent::update() {
	// -- updt shape
	mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
	mesh.clear();
	shape.clear();
	shape.begin();
	for (int i = 0; i < trail.size(); i++) {
		shape.addVertex(pos.x + trail[i].x, pos.y + trail[i].y, pos.z + trail[i].z);
	}
	shape.close();
	shape.end();
	ofTessellator().tessellateToMesh(shape, OF_POLY_WINDING_NONZERO, mesh);
	for (int i = 0; i < mesh.getNumVertices(); i++) {
		mesh.addColor(color);
	}
}

void Agent::draw() {
	ofSetColor(color);
	shape.draw();
	mesh.draw();
};


bool Agent::isInside(ofPoint p) {
	if (shape.inside(p)) {
		return true;
	} else {
		return false;
	}
};


void Agent::explode() {
	color = ofColor(255, ofMap(r, 10, 100, 255, 50));
	if (r < 100) {
		r+=5;
	} else { 
		isDead = true;
	}
}
	
void Agent::redoTrail() {
	nt = 222;
	trail.resize(0);
	ofPoint tPo = ofPoint(0, 0, 0);
	for (int i = 0; i < nt; i++) {
		tPo.x = r * cos(i*TWO_PI / nt);
		tPo.y = r * sin(i*TWO_PI / nt);
		trail.push_back(tPo);
	}
}