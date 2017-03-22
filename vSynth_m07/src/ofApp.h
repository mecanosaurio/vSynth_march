#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxImageSequenceRecorder.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"
#include "ofxMaxim.h"
#include "maxiMFCC.h"
#include "ofxPostGlitch.h"

#define OSC_IN_PORT 6666

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void exit();

	// ------ ------ ------ ------ ------ ------ | audio + maximilian stuff |
	void audioRequested(float * input, int bufferSize, int nChannels);
	void audioReceived(float * input, int bufferSize, int nChannels);

	float 	* lAudioOut;
	float   * rAudioOut;
	float * lAudioIn;
	float * rAudioIn;
	int		initialBufferSize;
	int		sampleRate;

	double wave, sample, outputs[2], ifftVal;
	maxiMix mymix;
	maxiOsc osc;

	ofxMaxiFFTOctaveAnalyzer oct;
	int nAverages;
	float *ifftOutput;
	int ifftSize;
	float peakFreq = 0;
	float centroid = 0;
	float RMS = 0;
	ofxMaxiIFFT ifft;
	ofxMaxiFFT mfft;
	int fftSize;
	int bins, dataSize;
	maxiMFCC mfcc;
	double *mfccs;
	maxiSample samp;
	// ------ ------ ------ ------ ------ ------ | data buffer stuff |
	vector< vector<float> > dataBuff;
	vector<float> d0;
	vector<float> mB;
	int dBuffSize;
	int nBands;
	int rot;
	float a_rms;
	// ------ ------ ------ ------ ------ ------ | gui stuff |
	bool showGui;
	ofxPanel gui;

	ofxGuiGroup mixerGroup;
	ofxToggle dsp_enable;
	ofxIntSlider ini_a2, dur2, vel2;
	ofxToggle dots_enable;
	ofxToggle autoCue, autoExt, autoColor, autoCam;

	ofxGuiGroup playGroup;
	ofxIntSlider ini_a1, dur1, vel1;

	ofxGuiGroup meshGroup;
	ofxFloatSlider extrusion, extrusound, mov_y;
	ofxToggle faces_enable;
	ofxToggle wiref_enable;
	ofxToggle bind_enable;

	ofxGuiGroup colorGroup;
	ofxIntSlider offs_x, offs_y, offs_z;

	ofxGuiGroup camGroup;
	ofxFloatSlider pan, roll, yaw, dista;


	ofxFloatSlider mov_z, lim_z, start_z, period;
	ofxToggle play;
	// ------ ------ ------ ------ ------ ------ | osc stuff |
	ofxOscReceiver receiver;
	int v1, v2;
	// ------ ------ ------ ------ ------ ------ | rec stuff |
	ofxImageSequenceRecorder recorder;
	bool recording;
	ofImage tImg;

	// ------ ------ ------ ------ ------ ------ | NRMLSTFF |


	float extrusionAmount;
	//ofVboMesh mesh;
	ofMesh mesh;
	ofVideoGrabber vidGrabber;

	//ofMesh mesh;
	int mw, mh, meshSize;
	
	vector<ofImage> seq_01;
	ofFbo fbo_01;
	ofPixels pixs;
	ofColor cColor;
	int index_01, index_02, ind_ff = 0;
	int fNo_01;
	int n_imgs_01;
	ofDirectory dir;
	int h, w, ih, iw, t, t1, t0, t2;
	float tt;
	bool nFlag;

	int ini1, ini2;

	// ------ ------ ------ ------ ------ ------ | cam0 |
	ofCamera cam;
	ofLight light;
	bool bOrbit, bRoll;
	
	
};
