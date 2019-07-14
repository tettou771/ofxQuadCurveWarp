#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	fbo.allocate(1280, 720);
	warper.setup();
	warper.setTexture(&fbo.getTexture());
	warper.setDivision(20, 20);
}

//--------------------------------------------------------------
void ofApp::update() {

}

//--------------------------------------------------------------
void ofApp::draw() {
	// make fbo
	fbo.begin();
	ofBackground(50);
	ofDrawLine(0, 0, fbo.getWidth(), fbo.getHeight());
	ofDrawLine(0, fbo.getHeight(), fbo.getWidth(), 0);
	fbo.end();

	// draw fbo with warper
	ofClear(0);
	warper.draw();

	// help message
	string helpMessage = "";
	if (warper.isEnableEdit()) {
		helpMessage += "Click any point and move to drag/cursorKey";
		helpMessage += "Right click to toggle edit mode\n";
		helpMessage += "\n";

		switch(warper.getEditmode()){
		case ofxQuadCurveWarp::EditMode::NoEdit: break;
		case ofxQuadCurveWarp::EditMode::MasterWarper:
			helpMessage += "Alternative selection, you can select by '1' - '4' key\n";
			break;
		case ofxQuadCurveWarp::EditMode::Flexible:
			helpMessage += "This mode can select with raw/column/edge/cornar based selection\n";
			helpMessage += "If you wanna select neighbor vertex, press Shift + cursorKey\n";
			break;
		case ofxQuadCurveWarp::EditMode::Gaussian:
			helpMessage += "This mode can select with distance\n";
			helpMessage += "If you wanna select neighbor vertex, press Shift + cursorKey\n";
			break;
		case ofxQuadCurveWarp::EditMode::Point:
			helpMessage += "This mode only can a point move\n";
			helpMessage += "If you wanna select neighbor vertex, press Shift + cursorKey\n";
			break;
		}
	}
	else {
		helpMessage = "Space key to begin warp control";
	}

	ofDrawBitmapString(helpMessage, 10, 10);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {

	case ' ':
		// toggle warper controller
		if (warper.isEnableEdit()) {
			warper.mouseKeyboardDisable();
		}
		else {
			warper.mouseKeyboardEnable();
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
