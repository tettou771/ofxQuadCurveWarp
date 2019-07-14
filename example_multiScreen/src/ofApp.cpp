#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	mainScreen.allocate(1920, 1080, GL_RGB, 8);
	multiScreenWarper.setup(&mainScreen, 3, 10, 10);
}

//--------------------------------------------------------------
void ofApp::update(){
	mainScreen.begin();
	ofBackground(50);
	ofDrawGrid(120, 16, false, false, false, true);
	mainScreen.end();

	multiScreenWarper.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofClear(0);

	multiScreenWarper.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case ' ':
		toggleWarperShowing();
		break;
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

void ofApp::toggleWarperShowing() {
	if (multiScreenWarper.getShowing()) {
		multiScreenWarper.setShowing(false);
		ofHideCursor();
	}
	else {
		multiScreenWarper.setShowing(true);
		ofShowCursor();
	}
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
