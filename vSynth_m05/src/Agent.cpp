#include "Agent.h"


Agent::Agent() {};
Agent::~Agent() {};


void Agent::setup(ofPoint position, float lon, ofColor co) {
	pos = position;
	vel = ofPoint(0, 0, 0);
	n = 100;
	seg = lon / n;
	line.clear();
	for (int i = 0; i < n; i++) {
		line.addVertex(pos.x + i*seg, pos.y, 0);
	}
	color = co;
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


void Agent::popWith(vector<float> mB, float sc) {
	for (int i = 0; i < n; i++) {
		line[i].z += mB[i] * sc;
	}
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
	float te = abs( (ofGetHeight()/2) - pos.y)/20;
	ofSetColor(color, 255 - te*te);
	line.draw();
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