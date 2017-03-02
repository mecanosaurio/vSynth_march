#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	ofBackground(0);

	w = ofGetWidth();
	h = ofGetHeight();
	fbo_a.allocate(w, h, GL_RGBA);
	fbo_a.begin(); ofClear(0, 0); fbo_a.end();
	p_a = ofPoint(0, 0, 0);
	//ofSetColor(255,0,0);
	ofSetLineWidth(4);
	//ofNoFill();

	//cells
	theCells.resize(0);
	for (int i = 0; i < 36-1; i++) {
		Cell tempCell;
		if (i > 11 && i < 23) {
			int nt = int(ofRandom(6, 16));
			tempCell.setup(ofPoint(ofRandom(w), 10 + i * 20, 0), nt);
			tempCell.v = ofRandom(3*PI/4, PI);
		} else {
			int nt = int(ofRandom(3, 11));
			tempCell.setup(ofPoint(ofRandom(w), 10 + i * 20, 0), nt);
		}
		theCells.push_back(tempCell);
	}

	//rec
	recording = false;
	recorder.setPrefix(ofToDataPath("rec1/frame_"));
	recorder.setFormat("jpg");
}

//--------------------------------------------------------------
void ofApp::update(){
	t = ofGetElapsedTimeMillis();
	tt = ofGetElapsedTimef();
	fbo_a.begin();
	ofEnableBlendMode(OF_BLENDMODE_SCREEN);
	ofClear(0, 0);
	for (int i = 0; i < theCells.size(); i++) {
		theCells[i].go_to();
		//if (i != 10 && i != 11 && i != 23 && i != 24) {
			ofSetColor(theCells[i].color);
			theCells[i].draw();
		//}
	}
	ofDisableBlendMode();
	fbo_a.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
	fbo_a.draw(0, 0);
	tImg.grabScreen(0, 0, w, h);
	if (recording) recorder.addFrame(tImg);
}

//--------------------------------------------------------------
void ofApp::exit() {
	recorder.waitForThread();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'r') {
		recording = !recording;
	}

	if (key == 't') {
		if (recorder.isThreadRunning()) {
			recorder.stopThread();
		}
		else {
			recorder.startThread(false, true);
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	for (int i = 0; i < 36 - 1; i++) {
		theCells[i].dest = ofPoint(w/2+ofRandom(-w/2, w/2), 10 + i * 20, 0);
		theCells[i].spc_td = ofRandom(0.8, 1.1) * w/theCells[i].nt;
		//theCells[i].cwd = ofRandom(0.01, 1.9) * w / theCells[i].nt;
		theCells[i].cwd = ofRandom(0.01, 1.9) * w / theCells[i].nt;
		
		if (i > 11 && i < 23) {
			float tv = ofRandom(0.5, 2);
			theCells[i].v = ofRandom(tv * 3 * PI / 4, tv*PI);
		} else {
			theCells[i].v = ofRandom(0.01, 0.5);
		}
	}
	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
