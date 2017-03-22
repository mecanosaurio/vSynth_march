#include "Agent.h"


Agent::Agent() {};
Agent::~Agent() {};


void Agent::setup(ofPoint position) {
	pos = position;
	vel = ofPoint(0, 0, 0);
	n = 2;
	r = 10;
	size = r;
	line.clear();
	line.addVertex(ofPoint(pos.x - r, pos.y, pos.z));
	line.addVertex(ofPoint(pos.x + r, pos.y, pos.z));
	color = ofColor::white;
	//color.lerp(ofColor::blue, ofRandom(0.5));
	isDead = false;
};


void Agent::setVel(ofPoint v) {
	vel.x = v.x;
	vel.y = v.y;
	vel.z = v.z;
}


void Agent::fall() {
	pos.x += vel.x;
	if (pos.x > ofGetWidth()) pos.x = 0;
	if (pos.x < 0) pos.x = ofGetWidth();
	pos.y += vel.y;
	if (pos.y > ofGetHeight()) isDead = true;
	if (pos.y < 0 ) pos.y = ofGetHeight();
	pos.z += vel.z;
	if (pos.z > 500) pos.z = 0;
	if (pos.z < -500) pos.z = 500;
};


void Agent::grow(float rate) {
	r += rate;
	if (r > 1000) isDead = true;
}


void Agent::reset() {
	line.clear();
	for (int i = 0; i < n; i ++) {
		line.addVertex(pos.x, pos.y + i*seg, 0);
	}
};


void Agent::amp(float factor) {
	//line.clear();
	line[0] = ofPoint(pos.x - r*factor, pos.y, factor * 53);
	line[1] = ofPoint(pos.x + r*factor, pos.y, factor * 53);
	size = 2 * r*factor;
};


void Agent::update() {
	// -- updt line and mesh
	line.clear();
	line.begin();
	for (int i = 0; i < trail.size(); i++) {
		line.addVertex(pos.x + trail[i].x, pos.y + trail[i].y, pos.z + trail[i].z);
	}
	line.close();
	line.end();

	mesh.setMode(OF_PRIMITIVE_LINE_LOOP);
	mesh.clear();
	ofTessellator().tessellateToMesh(line, OF_POLY_WINDING_NONZERO, mesh);
	for (int i = 0; i < mesh.getNumVertices(); i++) {
		mesh.addColor(color);
	}
};


void Agent::draw() {
	ofSetColor(color);
	line.draw();
};

void Agent::drawTrail() {
	ofSetColor(color);
	line.draw();
	ofPushMatrix();
	for (int ii = 0; ii < (ofGetWidth() / 2)+size ; ii+=(size+size/5)) {
		ofTranslate(size + +size / 5, 0, 0);
		line.draw();
	}
	ofPopMatrix();
	ofPushMatrix();
	for (int ii = 0; ii < (ofGetWidth() / 2) + size; ii += (size + size / 5)) {
		ofTranslate(-(size + +size / 5), 0, 0);
		line.draw();
	}
	ofPopMatrix();
};


bool Agent::isInside(ofPoint p) {
	if (line.inside(p)) {
		return true;
	} else {
		return false;
	}
};


void Agent::explode() {

};


void Agent::redoTrail() {
	for (int i = 0; i < n; i++) {
		line[i].y += vel.y;
	}
};