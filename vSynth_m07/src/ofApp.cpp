

#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(true);
	h = ofGetHeight();
	w = ofGetWidth();
	ofSetFrameRate(60);
	ofBackground(0);
	iw = 1280;
	ih = 720;
	// ... ... ... --- --- ... ... ... --- --- ... ... --- osc
	receiver.setup(OSC_IN_PORT);
	cout << "listening for osc messages on port " << OSC_IN_PORT << "\n";
	// ... ... ... --- --- ... ... ... --- --- ... ... --- | dsp |
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
	a_rms = 0;

	// ... ... ... --- --- ... ... ... --- --- ... ... --- | gui |
	showGui = true;
	gui.setup();

	mixerGroup.setup("miXXXer");
	mixerGroup.setHeaderBackgroundColor(ofColor::lightBlue);
	mixerGroup.setBorderColor(ofColor::lightSeaGreen);
	mixerGroup.add(dsp_enable.setup("dsp", false));
	mixerGroup.add(dots_enable.setup("dots", false));
	mixerGroup.add(autoCue.setup("autoPlay", false));
	mixerGroup.add(autoExt.setup("autoExtrusion", false));
	mixerGroup.add(autoColor.setup("autoColor", false));
	mixerGroup.add(autoCam.setup("autoCam", false));
	gui.add(&mixerGroup);

	playGroup.setup("player");
	playGroup.setHeaderBackgroundColor(ofColor::darkSlateBlue);
	playGroup.setBorderColor(ofColor::lightSlateGrey);
	playGroup.add(ini_a1.setup("ini", 10, 1, 100));
	playGroup.add(dur1.setup("dur", 10, 1, 500));
	playGroup.add(vel1.setup("vel", 48, 1, 200));
	gui.add(&playGroup);

	meshGroup.setup("mesh");
	meshGroup.setHeaderBackgroundColor(ofColor::darkTurquoise);
	meshGroup.setBorderColor(ofColor::lightSkyBlue);
	meshGroup.add(extrusion.setup("extrusion", 500, 0, 2500));
	meshGroup.add(extrusound.setup("extrusound", 40, 0, 200));
	meshGroup.add(bind_enable.setup("bind", false));
	meshGroup.add(faces_enable.setup("faces", false));
	meshGroup.add(wiref_enable.setup("wiref", false));
	gui.add(&meshGroup);

	colorGroup.setup("color");
	colorGroup.setHeaderBackgroundColor(ofColor::lightYellow);
	colorGroup.setBorderColor(ofColor::darkGreen);
	colorGroup.add(offs_x.setup("offs_x", 1, 0, w));
	colorGroup.add(offs_y.setup("offs_y", 1, 0, w));
	colorGroup.add(offs_z.setup("offs_z", 1, 0, w));
	gui.add(&colorGroup);

	camGroup.setup("camera");
	camGroup.setHeaderBackgroundColor(ofColor::green);
	camGroup.setBorderColor(ofColor::black);
	camGroup.add(pan.setup("pan", 0, 0, 360));
	camGroup.add(roll.setup("roll", 0, 0, 360));
	camGroup.add(yaw.setup("yaw", 90, 0, 360));
	camGroup.add(dista.setup("dista", 2000, 0, 3500));
	gui.add(&camGroup);



	// ... ... ... --- --- ... ... ... --- --- ... ... --- imgs

	fbo_01.allocate(w, h, GL_RGBA);
	fbo_01.begin(); ofClear(0, 0); fbo_01.end();

	n_imgs_01 = dir.listDir("salen");
	//n_imgs_01 = dir.listDir("imgs_casini");
	//n_imgs_01 = dir.listDir("sss");
	//n_imgs_01 = dir.listDir("eucfrag720");
	//dir.sort();
	n_imgs_01 = 1000;
	seq_01.resize(n_imgs_01);
	for (int i = 0; i < n_imgs_01; i++) {
		string fn = dir.getPath(i);
		//seq_01[i].allocate(iw, ih, OF_IMAGE_COLOR_ALPHA);
		seq_01[i].load(fn);
		cout << "Reading_IMGSSS: " << i << "/" << n_imgs_01 << "\n";
	}
	iw = seq_01[n_imgs_01 - 2].getWidth();
	ih = seq_01[n_imgs_01 - 2].getHeight();
	
	meshSize = 10;
	mw = iw/ meshSize;
	mh = ih/ meshSize;

	ini1 = (ini_a1/100)*n_imgs_01;
	index_01 = 500;
	nFlag = false;
	// ... ... ... --- --- ... ... ... --- --- ... ... --- mesh
	
	for (int y = 0; y < mh; y++) {
		for (int x = 0; x < mw; x++) {
			//mesh.addVertex(ofPoint(x, y, 0));
			mesh.addVertex(ofPoint((x - mw / 2) * meshSize, (y - mh / 2) * meshSize, 0)); // adding texure coordinates allows us to bind textures to it later // --> this could be made into a function so that textures can be swapped / updated
			mesh.addTexCoord(ofPoint(x * (iw / mw), y * (ih / mh)));
			mesh.addColor(ofColor(255,255,255));
		}
	}

	for (int y = 0; y < mh - 1; y++) {
		for (int x = 0; x < mw - 1; x++) {
			mesh.addIndex(x + y*mw);				// 0
			mesh.addIndex((x + 1) + y*mw);			// 1
			mesh.addIndex(x + (y + 1)*mw);			// 10

			mesh.addIndex((x + 1) + y*mw);			// 1
			mesh.addIndex((x + 1) + (y + 1)*mw);		// 11
			mesh.addIndex(x + (y + 1)*mw);			// 10

			/*
			int i1 = x + mw * y;
			int i2 = x + 1 + mw * y;
			int i3 = x + mw * (y + 1);
			int i4 = x + 1 + mw * (y + 1);
			mesh.addTriangle(i1, i2, i3);
			mesh.addTriangle(i2, i4, i3);
			*/
		}
	}
	
	//mainMesh.plane(1280, 720);
	extrusionAmount = extrusion;
	//mainMesh.setMode(OF_PRIMITIVE_TRIANGLES);

	// ... ... ... --- --- ... ... ... --- --- ... ... --- | cam |
	light.setAmbientColor(ofColor(255));
	light.enable();

	// ... ... ... --- --- ... ... ... --- --- ... ... --- | rec |
	recording = false;
	recorder.setPrefix(ofToDataPath("rec1/frame_"));
	recorder.setFormat("jpg");

	// .............
	/*


	*/

}


//--------------------------------------------------------------
void ofApp::update(){
	t = ofGetElapsedTimeMillis();
	tt = ofGetElapsedTimef();

	std::stringstream strm;
	strm << "vSynth [fps]: " << ofGetFrameRate();
	ofSetWindowTitle(strm.str());
	// ... ... ... --- --- ... ... ... --- --- ... ... --- osc
	while (receiver.hasWaitingMessages()) {
		ofxOscMessage m;
		receiver.getNextMessage(m);
		// --- mixer
		if (m.getAddress() == "/cc/1") {
			ini1 = m.getArgAsFloat(0)*n_imgs_01;
			cout << "[ini1]: " << ini1 << "\n";
		}
		else if (m.getAddress() == "/cc/2") {
			dur1 = m.getArgAsFloat(0) * 100;
			cout << "[dur1]: " << dur1 << "\n";
		}
		else if (m.getAddress() == "/cc/3") {
			vel1 = m.getArgAsFloat(0) * 100;
			cout << "[vel1]: " << vel1 << "\n";
		}
		if (m.getAddress() == "/cc/4") {
			ini2 = m.getArgAsFloat(0)*n_imgs_01;
			cout << "[ini2]: " << ini2 << "\n";
		}
		else if (m.getAddress() == "/cc/5") {
			dur2 = m.getArgAsFloat(0) * 100;
			cout << "[dur2]: " << dur2 << "\n";
		}
		else if (m.getAddress() == "/cc/6") {
			vel2 = m.getArgAsFloat(0) * 100;
			cout << "[vel2]: " << vel2 << "\n";
		}
	}
	// ... ... ... --- --- ... ... ... --- --- ... ... --- | dsp |
	if (dsp_enable == true) {
		for (int b = 0; b < nBands; b++) { if (oct.averages[b] > 0) { d0[b] = oct.averages[b]; } }
		for (int n = dBuffSize - 1; n > 0; n--) { dataBuff[n] = dataBuff[n - 1]; }
		dataBuff[0] = d0;
		for (int b = 0; b < nBands; b++) {
			mB[b] = 0;
			for (int n = 0; n < dBuffSize; n++) { mB[b] += dataBuff[n][b]; }
			mB[b] /= dBuffSize;
		}
		a_rms = ofLerp(a_rms, RMS, 0.3);
	}
	// ... ... ... --- --- ... ... ... --- --- ... ... --- tunneling
	ini1 = ini_a1*(n_imgs_01 / 100.0);
	ini2 = ini_a2*(n_imgs_01 / 100.0);
	if ((ini1+ind_ff)<n_imgs_01 - dur1 && (ini1+ind_ff)<ini1 + dur1) {
		t0 = ofGetElapsedTimeMillis();
		if ((t0 - t1) > vel1) {
			//index_01++;
			ind_ff++;
			index_01 = ini1 + ind_ff;
			nFlag = true;
			t1 = ofGetElapsedTimeMillis();
		}
	}
	else ind_ff = 0;
	// ... ... ... --- --- ... ... ... --- --- ... ... --- imgs
	/*
	fbo_01.begin();
	ofClear(0, 0);
	//ofEnableAlphaBlending();
	ofEnableBlendMode(OF_BLENDMODE_SCREEN);
	ofSetColor(255, 250);
	seq_01[index_01].draw(0, 0, w, h);

	ofSetColor(255, 250);
	seq_01[index_02].draw(0, 0, w, h);
	ofDisableBlendMode();
	//ofDisableAlphaBlending();
	fbo_01.end();
	*/
	// ... ... ... --- --- ... ... ... --- --- ... ... --- change states
	if (autoCue == true) { //, autoExt, autoColor, autoCam;
		if (a_rms > 2 && mB[44] > 3) {
			if (t % 60 == 0) ini_a1 = (ini_a1 > 5 && ini_a1 < 95) ? ini_a1 + 5 * ofNoise(t / 11) : ofRandom(40, 60);
			vel1 = ofRandom(24, 32);
			dur1 = ofRandom(4, 132);
		}
	}
	if (autoExt == true) {
		if (a_rms > 2 && mB[44] > 3) {
			extrusion = 800;
			extrusound = 20;
		}
		if (a_rms > 1 && mB[12] > 3) {
			extrusion = 700;
			extrusound = 20;
		}
		if (a_rms > 2 && mB[74] > 3) {
			extrusion = 1000;
			extrusound = 40;
		}
	}
	if (autoColor == true) {
		if (a_rms> 2 && mB[44]> 5) {
			offs_x = (offs_x < w / 7) ? offs_x + 1 : 0;
			offs_y = (offs_y < w / 7) ? offs_y + 1 : 0;
			offs_y = (offs_z < w / 7) ? offs_z + 1 : 0;
		}
		else {
			offs_x = offs_x + ofRandom(-1.1, 2.1); offs_y = offs_y + ofRandom(-1.1, 2.1); offs_z = offs_z + ofRandom(-1, 1);
		}
	}
	if (autoCam == true) {
		if (a_rms > 2 && mB[44] > 2) {
			pan = (t % 120 >90) ? 90 : 0; (t % 150 >90) ? 270 : 0;
		}
		if (a_rms > 2 && mB[90] > 2) {
			yaw = 0; pan = ofRandom(90, 270);
			roll = (t % 120 >90) ? 90 : 0;
			dista = 200;
		}
		if (a_rms > 1 && mB[12] > 2) {
			yaw = 0; roll = 90;
			pan = (t % 120 >90) ? 90 : 0;
			dista = 1500;
		}
	}
	/*
	if (a_rms> 2 && mB[22] > 5) {
		if(t%180<40)	ini_a1 = (ini_a1 > 5 && ini_a1 < 95) ? ini_a1 + ofRandom(1.5) * ofNoise(t / 21) : ofRandom(80, 60);
		//vel1 = 4;
		//dur1 = 10;
		wiref_enable = (t % 11 < 3) ? true : false;
		bind_enable = false;
		faces_enable = (t % 11 < 3) ? false: true;
		yaw = 0; pan = 0; roll = 0;
		extrusion = 800;
		extrusound = 20;
		dista = (t % 240>200) ? 1000 : ofRandom(1000, 1200);
	} 
	if (a_rms> 2 && mB[44]> 5) {
		if (t % 60 == 0) ini_a1 = (ini_a1 > 5 && ini_a1 < 95) ? ini_a1 + 5*ofNoise(t/11) : ofRandom(40, 60);
		vel1 = ofRandom(24, 32);
		dur1 = ofRandom(4, 132);
		pan = (t % 120 >90) ? 90 : 0; (t % 150 >90) ? 270 : 0;
		offs_x = (offs_x < w / 7) ? offs_x + 1 : 0; 
		offs_y = (offs_y < w / 7) ? offs_y + 1 : 0;
		offs_y = (offs_z < w / 7) ? offs_z + 1 : 0;
	} 
	if (a_rms> 2 && mB[58] > 4) {
		dur1 = 100 + 1 * ofRandom(-10, 10);
		extrusion = 700;
		extrusound = 20 ;
		wiref_enable = (t % 70 < 20) ? true: false;
		bind_enable = (t % 110 > 80) ? false : true;;
		roll = (t % 120 >90) ? 90 : 0;
		dista = (t%240<100) ? ofRandom(1000, 1200):2000;
		offs_x = offs_x+ofRandom(-3,3); offs_y = offs_y +ofRandom(-2,2); offs_z = offs_z + ofRandom(-2,2);
	} 
	if (a_rms> 2 && mB[72] > 4) {
		//vel1 = ofRandom(5, 25);
		//dur1 = ofRandom(2, 40);
		bind_enable = true;
		extrusion = 900;
		extrusound = 40;
		yaw = 0; roll = 90;
		dista = int(ofRandom(8,16))*100;
	} 
	if (a_rms> 2 && mB[86] > 3) {
		extrusion = 1500;
		extrusound = 80;
		pan = 90*ofRandom(5); 
		dista = 1200;
	} else {
		//wiref_enable = (t % 70 < 20 || t%640 < 50) ? true : false;
		//yaw = 0; pan = (t % 120 >90) ? 90 : 0; roll = 0;
	}
	*/
	// ... ... ... --- --- ... ... ... --- --- ... ... --- mesh-/-thing
	if (nFlag) {
		//fbo_01.readToPixels(pixs);
		pixs = seq_01[index_01].getPixels();
		for (int y = 0; y < mh; y++) {
			for (int x = 0; x < mw; x++) {

				//Vertex index
				int i = x + mw * y;
				//ofPoint p = mesh.getVertex(i);

				float scaleX = iw / mw;
				float scaleY = ih / mh;

				// get brightness
				int index = int((x * scaleX) + iw * (y * scaleY)) ; //
				ofFloatColor cColor(pixs[3*index+offs_z] / 255.f, 			// r
									pixs[3*index+1+offs_x] / 255.f,			// g
									pixs[3*index+2+offs_y] / 255.f);			// b

				ofVec3f tmpVec = mesh.getVertex(i);
				float mm = MIN(a_rms, 1);
				tmpVec.z = ofLerp(tmpVec.z, cColor.getBrightness() * extrusion, 0.5) + mm * extrusound;// ;
				mesh.setVertex(i, tmpVec);
				float bb = cColor.getBrightness();
				mesh.setColor(i, ofColor(cColor, 127+int (mm*150)));
				/*
				int brightness = pixs[index];
													   //Change z-coordinate of vertex
				p.z = brightness*2;//+20 * ofNoise(x * 0.05, y * 0.05, ofGetElapsedTimef() * 0.5);
				//p.z = ofNoise(x * 0.05, y * 0.05, ofGetElapsedTimef() * 0.5) * 100;
				mesh.setVertex(i, p);

				//Change color of vertex
				//mesh.setColor(i, pixs[index]);
				*/
			}
		}
		nFlag = false;
		if(dots_enable==true) mesh.setMode(OF_PRIMITIVE_POINTS);
		else mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	}
	// ... ... ... --- --- ... ... ... --- --- ... ... --- cam
	// here's where the transformation happens, using the orbit and roll member functions of the ofNode class, 
	// since angleH and distance are initialised to 0 and 500, we start up as how we want it
	cam.orbit(pan, yaw, dista);
	cam.roll(roll);
}

//--------------------------------------------------------------
void ofApp::draw(){
	//ofBackgroundGradient(ofColor(150), ofColor::black);
	ofEnableDepthTest();
	cam.begin();
	if ((RMS > 1) && t%8 <3 || (t % 7 < 5 || mB[12] > 2 || t % 5 < 2) && RMS<1) {
		if (bind_enable == true) seq_01[index_01].bind();
		if (faces_enable == true) mesh.drawFaces();
		else if (wiref_enable == true) mesh.drawWireframe();
		else mesh.draw();
		if (bind_enable == true) seq_01[index_01].unbind();
	}
	cam.end();
	//fbo_01.draw(0,0);
	ofDisableDepthTest();

	// r + sh
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
	switch(key) {
		case 'f':
			ofToggleFullscreen();
			break;
		case 'h':
			showGui = !showGui;
			break;
		case 'r':
			recording = !recording;
			break;
		case 't':
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