#ifndef __ofxQuadCurveWarpSettings_h__
#define __ofxQuadCurveWarpSettings_h__

#pragma once

#include "ofMain.h"
#include "ofxQuadCurveWarp.h"

class ofxQuadCurveWarp;

class ofxQuadCurveWarpSettings {
	friend ofxQuadCurveWarp;

public:
	ofxQuadCurveWarpSettings();
	~ofxQuadCurveWarpSettings();

	void getSettingsFrom(ofxQuadCurveWarp &warper);
	void applySettingsTo(ofxQuadCurveWarp &warper);

	void save(string fileName = "ofxQuadCurveWarpSettings");
	void load(string fileName = "ofxQuadCurveWarpSettings");

	bool isLoaded() { return loaded; }

private:
	int divisionX, divisionY;
	ofRectangle sourceRect;
	vector<ofPoint> masterWarperTargetPoints;
	vector<ofVec2f> targetOffsets;

	bool loaded;
};

#endif