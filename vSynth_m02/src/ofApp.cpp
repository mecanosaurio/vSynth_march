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
	mixerGroup.add(tog_a.setup("t_1", false));
	mixerGroup.add(slid_a.setup("s_A", 255, 0, 255));
	mixerGroup.add(slid_b.setup("s_B", 255, 0, 255));
	mixerGroup.add(slid_c.setup("s_C", 155, 0, 512));
	mixerGroup.add(slid_d.setup("s_D", 0.75, 0, 1));
	mixerGroup.add(tog_b.setup("t_2", false));
	gui.add(&mixerGroup);

	// ------ ------ ------ ------ ------ ------ | more stuff |
	fbo_a.allocate(w, h, GL_RGBA);
	fbo_a.begin(); ofClear(0, 0); fbo_a.end();
	p_a = ofPoint(0, 0, 0);
	//ofSetColor(255,0,0);
	ofSetLineWidth(4);
	//ofNoFill();

	// ------ ------ ------ ------ ------ ------ | cells |
	theCells.resize(0);
	for (int i = 0; i < 36-1; i++) {
		Cell tempCell;
		if (i > 11 && i < 23) {
			int nt = int(ofRandom(6, 16));
			tempCell.setup(ofPoint(ofRandom(w), 10 + i * 20, 0), nt);
			tempCell.v = ofRandom(PI/8, PI/6);
		} else {
			int nt = int(ofRandom(3, 11));
			tempCell.setup(ofPoint(ofRandom(w), 10 + i * 20, 0), nt);
		}
		theCells.push_back(tempCell);
	}

	// ------ ------ ------ ------ ------ ------ | rec |
	recording = false;
	recorder.setPrefix(ofToDataPath("rec1/frame_"));
	recorder.setFormat("jpg");
	
	ofLoadImage(pix, "img_s06.jpg");
	posC = ofPoint(0,0,0);
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
	fbo_a.begin();
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofClear(0, 0);

	for (int i = 0; i < theCells.size(); i++) {
		theCells[i].go_to();
		//pos = int(ofRandom(pix.size()));
		posC.x += 5 * ofMap(ofNoise(tt), 0,1, -2,2);
		posC.y += 5 * ofMap(ofNoise(t/3), 0, 1, -2, 2);
		if (posC.x > 1000) posC.x -= 1000;
		if (posC.x < 0) posC.x += 1000;
		if (posC.y > 1200) posC.y -= 1200;
		if (posC.y < 0) posC.y += 1200;
		int ipp = (3 * (posC.y * 1023 + posC.x));
		iColor = pix.getColor(posC.x, posC.y);
		//	ofColor(pix[ipp], pix[ipp+1], pix[ipp+1], 100);
			
		theCells[i].color = iColor;
		//ofSetColor(iColor);
		theCells[i].cwd = (mB[12 + (4*i) % 80]/3) * w / theCells[i].nt;
		if (i % 4 == 0 && t % 17 < 7) {
			theCells[i].cH = -int(mB[24 + i % 24]) * 20;
		} else {
			theCells[i].cH = 20;
		}
		//if (mB[22]>5) {
		if(t%7>5 || t%25  <2 || d0[22]>5 || d0[60]>4 || d0[86]>3){
			theCells[i].draw();
		}
	}
	ofDisableBlendMode();
	fbo_a.end();
}

//--------------------------------------------------------------
void ofApp::draw(){
	fbo_a.draw(0, 0);
	tImg.grabScreen(0, 0, w, h);
	if (recording) recorder.addFrame(tImg);
	if (showGui) {
		gui.draw();
	}
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
	if (key == '5') 	ofLoadImage(pix, "img_s05.jpg");
	if (key == '6') 	ofLoadImage(pix, "img_s06.jpg");
	if (key == '7') 	ofLoadImage(pix, "img_s07.jpg");
	if (key == '8') 	ofLoadImage(pix, "img_s08.jpg");
	if (key == '9') 	ofLoadImage(pix, "img_s09.jpg");
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
			theCells[i].v = ofRandom(PI / 8, PI / 6);
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