#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(60);
	ofBackground(0);
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
	dBuffSize = 64;
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
	mixerGroup.add(tog_a.setup("t_1", false));
	mixerGroup.add(slid_a.setup("s_A", 0, -1, 1));
	mixerGroup.add(slid_b.setup("s_B", 0, -1, 1));
	mixerGroup.add(slid_c.setup("s_C", 155, 0, 512));
	mixerGroup.add(slid_d.setup("s_D", 0.75, 0, 1));
	mixerGroup.add(tog_b.setup("t_2", false));
	gui.add(&mixerGroup);

	// ------ ------ ------ ------ ------ ------ | more stuff |
	fbo_a.allocate(w, h, GL_RGBA);
	fbo_a.begin(); ofClear(0,0); fbo_a.end();
	p_a = ofPoint(0, 0, 0);
	//ofSetColor(255,0,0);
	ofSetLineWidth(4);
	//ofNoFill();

	// ------ ------ ------ ------ ------ ------ | Agents |
	theAgents.resize(0);
	for (int i = 0; i < 1; i++) {
		Agent tempAgent;
		tempAgent.setup(ofPoint(w/2, h/2, 700));
		theAgents.push_back(tempAgent);
	}

	// ------ ------ ------ ------ ------ ------ | rec |
	recording = false;
	recorder.setPrefix(ofToDataPath("rec1/frame_"));
	recorder.setFormat("jpg");
	
	ofLoadImage(pix, "img_s09.jpg");
	posC = ofPoint(0,0,0);
	newcolor = ofColor(255);
}

//--------------------------------------------------------------
void ofApp::update(){
	t = ofGetElapsedTimeMillis();
	tt = ofGetElapsedTimef();

	std::stringstream strm;
	strm << "vSynth [fps]: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());

	// ------ ------ ------ ------ ------ ------ | dsp |
	if (tog_a) {
		for (int b = 0; b < nBands; b++) { if (oct.averages[b] > 0) { d0[b] = oct.averages[b]; } }
		for (int n = dBuffSize - 1; n > 0; n--) { dataBuff[n] = dataBuff[n - 1]; }
		dataBuff[0] = d0;
		for (int b = 0; b < nBands; b++) {
			mB[b] = 0;
			for (int n = 0; n < dBuffSize; n++) { mB[b] += dataBuff[n][b]; }
			mB[b] /= dBuffSize;
		}
	}

	// ------ ------ ------ ------ ------ ------ | other stuff |
	// -if time, add agent
	if (t - t0 >450) {
		Agent tempAgent;
		tempAgent.setup(ofPoint(w / 2, h / 2, 700));
		theAgents.push_back(tempAgent);
		t0 = t;
	}
	if (t - t1 > 10) {
		ofNode on = cam.getTarget();
		ofPoint ppp = ofPoint(on.getX(), on.getY(), on.getZ());
		if (t % 2 == 0) {
			ppp.x = 20 * int(ofRandom(-2.2, 2.2));
		}
		else {
			ppp.y = 20 * int(ofRandom(-2.2, 2.2));
		}
		cam.setTarget(ofPoint(ppp.x, ppp.y, -500));
		t1 = t;
	}
	// -make them move and chjek
	for (int i = 0; i < theAgents.size(); i++) {
		if (!theAgents[i].isDead) {
			if (!theAgents[i].isDown) {
				theAgents[i].fall(slid_a * w/2, slid_b * h/2);
			}
			else {
				theAgents[i].grow(4);
				theAgents[i].redoTrail();
			}
			theAgents[i].update();
		}
		else {
			theAgents.erase(theAgents.begin()+i);
		}
	}
	fbo_a.begin();
	cam.begin();
	ofPushMatrix();
	ofTranslate(-w / 2, -h / 2);
	ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
	//ofEnableAlphaBlending();
	ofClear(newcolor);

	for (int i = 0; i < theAgents.size(); i++) {
		theAgents[i].draw();
	}
	ofDisableBlendMode();
	//ofDisableAlphaBlending();
	ofPopMatrix();
	cam.end();
	fbo_a.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
	fbo_a.draw(0, 0);
	tImg.grabScreen(0, 0, w, h);
	if (recording) recorder.addFrame(tImg);
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
		newcolor = ofColor(ofRandom(205, 255), ofRandom(2, 255), ofRandom(12, 255), 255);
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