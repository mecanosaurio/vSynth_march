#ifndef _CELL
#define _CELL
#include "ofMain.h"


class Cell{
public:
	Cell();
	~Cell();
	void setup(ofPoint position, int ntr);
	void go_to();
	void update(float tt);
	void draw();
	bool isInside(ofPoint p);
	void fall();
	void reset();
	void explode();

	ofPoint pos;
	ofPoint dest;

	vector<ofPoint> trail;
	int spc_trail, spc_td, nt;
	ofPolyline shape;
	float a, b, c, r, v;
	float gravity;
	int n;
	ofColor color;
	bool isDead;
	bool isExpl;
	int cW, cH, cwd;
private:
};

#endif