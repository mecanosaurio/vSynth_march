#pragma once

#include "ofMain.h" 
#include "Cell.h"
#include "ofxImageSequenceRecorder.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"
#include "ofxMaxim.h"
#include "maxiMFCC.h"
#include "ofxPostGlitch.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
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

		// ------ ------ ------ ------ ------ ------ | gui stuff |
		bool showGui;
		ofxPanel gui;

		ofxGuiGroup mixerGroup;
		ofxToggle tog_a;
		ofxFloatSlider slid_a, slid_b;
		ofxFloatSlider slid_c, slid_d;
		ofxToggle tog_b;

		// ------ ------ ------ ------ ------ ------ | regular stuff |
		ofFbo fbo_a;
		ofPoint p_a;
		int w, h, bw, bh, t;
		float tt;

		vector <Cell> theCells;
		ofPoint dests[72];
		Cell *tempCell;

		ofxImageSequenceRecorder recorder;
		bool recording;
		ofImage tImg;

		ofPixels pix;
		ofPoint posC;
		ofColor iColor;

};
