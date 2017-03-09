#ifndef _AGENT
#define _AGENT
#include "ofMain.h"


class Agent{
public:
	Agent();
	~Agent();
	void setup(ofPoint position);
	void go_to();
	void update();
	void draw();
	bool isInside(ofPoint p);
	void fall(float g, float a, float d);
	void grow(float rate);
	void reset();
	void explode();
	
	void redoTrail(int lt);

	ofPoint pos;
	ofPoint dest;

	vector<ofPoint> trail;
	int spc_trail, spc_td, nt;
	ofPolyline shape;
	ofMesh mesh;
	ofTessellator tess;
	float a, b, c, r, v;
	float gravity, atract, dwell;
	int n;
	ofColor color;
	bool isDead, isDown;
	bool isExpl;
	int cW, cH, cwd;
private:
};

#endif