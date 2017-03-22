#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(30);
	ofBackground(0);
	//ofBackgroundGradient(ofColor::black, ofColor::blue);
	w = ofGetWidth();
	h = ofGetHeight();
	
	// ------ ------ ------ ------ ------ ------ | dsp |
	sampleRate = 44100;
	initialBufferSize = 512;
	lAudioOut = new float[initialBufferSize];
	rAudioOut = new float[initialBufferSize];
	lAudioIn = new float[initialBufferSize];
	rAudioIn = new float[initialBufferSize];
	memset(lAudioOut, 0, initialBufferSize * sizeof(float));
	memset(rAudioOut, 0, initialBufferSize * sizeof(float));
	memset(lAudioIn, 0, initialBufferSize * sizeof(float));
	memset(rAudioIn, 0, initialBufferSize * sizeof(float));

	fftSize = 1024;
	mfft.setup(fftSize, 512, 256);
	ifft.setup(fftSize, 512, 256);
	nAverages = 12;
	oct.setup(sampleRate, fftSize / 2, nAverages);
	mfccs = (double*)malloc(sizeof(double) * 13);
	mfcc.setup(512, 42, 13, 20, 20000, sampleRate);
	ofxMaxiSettings::setup(sampleRate, 2, initialBufferSize);
	ofSoundStreamSetup(2, 2, this, sampleRate, initialBufferSize, 4); // Call this last ! 
	// fft :: 512 , mfft.magnitudes[i] , pitch/timbral/vol
	// MFCC :: 13 , mfccs[i] , timbre/vocal
	// CosQ :: 104, oct.averages[i] , 12 bands/oct
	// peak :: peakFreq, spectral centroid :: centroid, rms::RMS
	nBands = 104;
	dBuffSize = 16;
	d0.resize(nBands);
	mB.resize(nBands);
	dataBuff.resize(dBuffSize);
	for (int n = 0; n < dBuffSize; n++) { dataBuff[n].resize(nBands); }


	// ------ ------ ------ ------ ------ ------ | gui |
	showGui = true;
	gui.setup();

	mixerGroup.setup("miXXXer");
	mixerGroup.setHeaderBackgroundColor(ofColor::blue);
	mixerGroup.setBorderColor(ofColor::blue);
	mixerGroup.add(dsp_enable.setup("dsp", false));
	mixerGroup.add(mov_y.setup("mov_Y", 0, -10, 10));
	mixerGroup.add(mov_x.setup("mov_X", 0, -50, 50));
	mixerGroup.add(mov_z.setup("mov_Z", 0, -10, 10));
	mixerGroup.add(start_z.setup("start_Z", 0, -2000, 700));
	mixerGroup.add(lim_z.setup("lim_Z", 1000, -2500, 900));
	mixerGroup.add(play.setup("play", false));
	mixerGroup.add(period.setup("period", 50, 2, 100));
	gui.add(&mixerGroup);

	kaleidoGroup.setup("kaleidoscope");
	kaleidoGroup.setHeaderBackgroundColor(ofColor::darkRed);
	kaleidoGroup.setBorderColor(ofColor::darkRed);
	kaleidoGroup.add(kenabled.setup("kaleidoEnabled", false));
	kaleidoGroup.add(ksectors.setup("ksectors", 4, 1, 23));
	kaleidoGroup.add(kangle.setup("kangle", 101.02, -180, 180));
	kaleidoGroup.add(kx.setup("kx", 0.459184, 0, 1));
	kaleidoGroup.add(ky.setup("ky", 0.673469, 0, 1));
	gui.add(&kaleidoGroup);

	// ------ ------ ------ ------ ------ ------ | fbo |
	fbo_a.allocate(w, h, GL_RGBA);
	fbo_a.begin(); ofClear(0,0); fbo_a.end();
	fbo_glitch.allocate(w, h, GL_RGBA);
	fbo_glitch.begin(); ofClear(0, 0); fbo_glitch.end();
	fbo4a.allocate(w, h, GL_RGBA32F_ARB);
	fbo4a.begin(); ofClear(0, 0); fbo4a.end();
	fbo4b.allocate(w, h, GL_RGBA);
	fbo4b.begin(); ofClear(0, 0); fbo4b.end();

	// ------ ------ ------ ------ ------ ------ | shaders |
	shader_kale.load("kaleido");
	myGlitch.setup(&fbo4a);
	//myGlitch.setFx(OFXPOSTGLITCH_CONVERGENCE, true);

	// ------ ------ ------ ------ ------ ------ | Agents |
	nblocks = 1;
	nAgents = 50;
	bw = w;
	bh = 400/nAgents;
	theAgents.resize(0);
	for (int i = 0; i < nAgents; i++) {
		Agent tempAgent;
		tempAgent.setup(ofPoint(w/2, 160 + i*bh, 0));
		theAgents.push_back(tempAgent);
	}
	
	// ------ ------ ------ ------ ------ ------ | rec |
	recording = false;
	recorder.setPrefix(ofToDataPath("rec1/frame_"));
	recorder.setFormat("jpg");
	
	ofLoadImage(pix, "im2.jpg");
	posC = ofPoint(0,0,0);
	newcolor = ofColor(255);

	// ------ ------ ------ ------ ------ ------ | colors |
	//ofSetColor(0, 255, 0, 200); 
	color2 = ofColor(89, 0, 254);
	color1 = ofColor(254, 0, 55);
	ofSetLineWidth(4);
	posC = ofPoint(500, 500, 0);
	//ofNoFill();
}

//--------------------------------------------------------------
void ofApp::update(){
	t = ofGetElapsedTimeMillis();
	tt = ofGetElapsedTimef();

	std::stringstream strm;
	strm << "vSynth [fps]: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());

	// ------ ------ ------ ------ ------ ------ | dsp |
	if (dsp_enable==true) {
		for (int b = 0; b < nBands; b++) { if (oct.averages[b] > 0) { d0[b] = oct.averages[b]; } }
		for (int n = dBuffSize - 1; n > 0; n--) { dataBuff[n] = dataBuff[n - 1]; }
		dataBuff[0] = d0;
		for (int b = 0; b < nBands; b++) {
			mB[b] = 0;
			for (int n = 0; n < dBuffSize; n++) { mB[b] += dataBuff[n][b]; }
			mB[b] /= dBuffSize;
		}
	}

	// ------ ------ ------ ------ ------ ------ | agents |
	//period = ofMap(centroid, 0, 0.6, 50, 10);
	//float peak = ofMap(RMS, 0.1, 10, 10, 5);
	/*
	if (t - t0 > period && play == true) {
		// set color
		posC.x += 5 * ofMap(ofNoise(tt), 0, 1,-2, 2);
		posC.y += 5 * ofMap(ofNoise(tt * 30), 0, 1, -2, 2);
		if (posC.x > 1000) posC.x -= 1000;
		if (posC.x < 0) posC.x += 1000;
		if (posC.y > 1000) posC.y -= 1000;
		if (posC.y < 0) posC.y += 1000;
		iColor = pix.getColor(posC.x, posC.y);
		// set agent
		Agent tempAgent;
		tempAgent.setup(ofPoint(0, 0, 200), bw+10, iColor);
		tempAgent.popWith(mB, 5);
		tempAgent.vel.y = 1;
		//tempAgent.color = ofColor();
		theAgents.push_back(tempAgent);
		t0 = t;
	}
	*/
	for (int i = 0; i < theAgents.size(); i++) {
		if (mB[i * 2] > 0.5) {
			theAgents[i].amp(mB[i * 2]/2);
		}
		else {
			theAgents[i].amp(0);
		}
	}
	
	
	// ------ ------ ------ ------ ------ ------ | other stuff |
	fbo_a.begin();
	ofClear(0, 0);
	//ofEnableBlendMode(OF_BLENDMODE_SCREEN);
	//ofEnableAlphaBlending();
	for (int i = 0; i < theAgents.size(); i++) {
		if (mB[2 * i]>0.5) theAgents[i].drawTrail();
		else theAgents[i].draw();
	}
	//ofDisableAlphaBlending();
	//ofDisableBlendMode();
	fbo_a.end();

	// ------ ------ ------ ------ ------ ------ | shaders |
	fbo4a.begin();
	ofClear(0, 0);
	shader_kale.begin();
	shader_kale.setUniform1i("ksectors", ksectors);
	shader_kale.setUniform1f("kangleRad", ofDegToRad(kangle));
	shader_kale.setUniform2f("kcenter", kx*w, ky*h);
	shader_kale.setUniform2f("screenCenter", 0.5*w, 0.5*h);
	fbo_a.draw(0,0);
	shader_kale.end();
	fbo4a.end();
	if (RMS > 3)ksectors = int(ofRandom(4, 9)); 
	/*if (RMS > 5) {
		myGlitch.setFx(OFXPOSTGLITCH_GLOW, false);
		myGlitch.setFx(OFXPOSTGLITCH_SHAKER, true);
		myGlitch.setFx(OFXPOSTGLITCH_OUTLINE, false);
	}*/
	if (RMS > 2) {
		myGlitch.setFx(OFXPOSTGLITCH_GLOW, true);
		myGlitch.setFx(OFXPOSTGLITCH_SHAKER, true);
		myGlitch.setFx(OFXPOSTGLITCH_OUTLINE, false);
	}
	else if (RMS > 1) {
		myGlitch.setFx(OFXPOSTGLITCH_GLOW, true);
		myGlitch.setFx(OFXPOSTGLITCH_SHAKER, true);
		myGlitch.setFx(OFXPOSTGLITCH_OUTLINE, true);
	}
	else if (RMS > 0.5) {
		myGlitch.setFx(OFXPOSTGLITCH_GLOW, true);
		myGlitch.setFx(OFXPOSTGLITCH_SHAKER, false);
		myGlitch.setFx(OFXPOSTGLITCH_OUTLINE, true);
	}
	else if (RMS > 0.1) {
		myGlitch.setFx(OFXPOSTGLITCH_GLOW, false);
		myGlitch.setFx(OFXPOSTGLITCH_SHAKER, false);
		myGlitch.setFx(OFXPOSTGLITCH_OUTLINE, true);
	}
	else {
		myGlitch.setFx(OFXPOSTGLITCH_GLOW, false);
		myGlitch.setFx(OFXPOSTGLITCH_SHAKER, false);
		myGlitch.setFx(OFXPOSTGLITCH_OUTLINE, false);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSetVerticalSync(true);
	myGlitch.generateFx();
	fbo4a.draw(0, 0);

	if (recording) {
		tImg.grabScreen(0, 0, w, h);
		recorder.addFrame(tImg);
	}
	if (showGui) gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
	gui.saveToFile("settings.xml");
	recorder.waitForThread();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'h') {
		showGui = !showGui;
	}
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
	if (key == ' ') {
		for (int i = 0; i < theAgents.size(); i++) {
			theAgents[i].redoTrail();
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
	if (button == 0) {
		newcolor = ofColor(ofRandom(205, 255), ofRandom(152, 255), ofRandom(202, 255), 255);
	}
	else {
		newcolor = ofColor(255);
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

//--------------------------------------------------------------
void ofApp::audioRequested(float * output, int bufferSize, int nChannels) {
	for (int i = 0; i < bufferSize; i++) {
		wave = lAudioIn[i];
		if (mfft.process(wave)) {
			mfft.magsToDB();
			oct.calculate(mfft.magnitudesDB);

			float sum = 0;
			float maxFreq = 0;
			int maxBin = 0;
			for (int i = 0; i < fftSize / 2; i++) {
				sum += mfft.magnitudes[i];
				if (mfft.magnitudes[i] > maxFreq) {
					maxFreq = mfft.magnitudes[i];
					maxBin = i;
				}
			}
			centroid = sum / (fftSize / 2);
			peakFreq = (float)maxBin / fftSize * 44100;

			mfcc.mfcc(mfft.magnitudes, mfccs);
		}
		lAudioOut[i] = 0;
		rAudioOut[i] = 0;
	}
}


//--------------------------------------------------------------
void ofApp::audioReceived(float * input, int bufferSize, int nChannels) {
	/* You can just grab this input and stick it in a double, then use it above to create output*/
	float sum = 0;
	for (int i = 0; i < bufferSize; i++) {
		/* you can also grab the data out of the arrays*/
		lAudioIn[i] = input[i * 2];
		rAudioIn[i] = input[i * 2 + 1];

		sum += input[i * 2] * input[i * 2];
	}
	RMS = sqrt(sum);
}