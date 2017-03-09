#ifndef _AGENT
#define _AGENT
#include "ofMain.h"


class Agent{
public:
	Agent();
	~Agent();
	void setup(ofPoint position, float lon, ofColor co);
	void popWith(vector<float> mB, float sc);
	void setVel(ofPoint v);
	void update();
	void draw();
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
	float r, seg;
	int n;
	bool isDead, isDown;

	vector<ofPoint> trail;
	ofTessellator tess;
	float a, b, c;
	bool isExpl;
private:
};

#endif