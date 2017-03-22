#ifndef _AGENT
#define _AGENT
#include "ofMain.h"


class Agent{
public:
	Agent();
	~Agent();
	void setup(ofPoint position);
	void amp(float factor);
	void setVel(ofPoint v);
	void update();
	void draw();
	void drawTrail();
	bool isInside(ofPoint p);
	void fall();
	void grow(float rate);
	void reset();
	void explode();
	
	void redoTrail();

	ofPoint pos, vel;
	ofPolyline line;
	ofMesh mesh;
	ofColor color;
	float r, seg, size;
	int n;
	bool isDead, isDown;

	vector<ofPoint> trail;
	ofTessellator tess;
	float a, b, c;
	bool isExpl;
private:
};

#endif