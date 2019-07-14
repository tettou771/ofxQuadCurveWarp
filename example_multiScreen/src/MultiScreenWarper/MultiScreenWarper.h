#pragma once
#include "ofMain.h"
#include "ofxQuadCurveWarp.h"
#include "ofxUI.h"

class WarpingFbo {
public:
	WarpingFbo(string _name, ofRectangle _targetRect, int divisionX, int divisionY) {
		warper.setup(_name);
		warper.setTargetRect(_targetRect);
		warper.setDivision(divisionX, divisionY);
	}

	void updateSubsectionScreen(ofFbo *source, ofFbo *blendingGradiation) {
		subsectionScreen.begin();
		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		source->getTexture().drawSubsection(0, 0, sourceRect.width, sourceRect.height, sourceRect.x, sourceRect.y);

		ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
		if (blendingWidthL > 0) {
			blendingGradiation->draw(blendingWidthL, 0, -blendingWidthL, subsectionScreen.getHeight());
		}
		if (blendingWidthR > 0) {
			blendingGradiation->draw(subsectionScreen.getWidth() - blendingWidthR, 0, blendingWidthR, subsectionScreen.getHeight());
		}

		subsectionScreen.end();
	}

	void draw() {
		warper.draw(&subsectionScreen.getTexture());
	}

	void setSourceRect(ofRectangle _sourceRect) {
		sourceRect = _sourceRect;
		if (sourceRect.width != subsectionScreen.getWidth() || sourceRect.height != subsectionScreen.getHeight()) {
			subsectionScreen.allocate(sourceRect.width, sourceRect.height, GL_RGB, 1);
			warper.setTexture(&subsectionScreen.getTexture());
			warper.setDivision(warper.getDivisionX(), warper.getDivisionY());
		}
	}

	void setBlendingWidthL(float _width) { blendingWidthL = _width; }
	void setBlendingWidthR(float _width) { blendingWidthR = _width; }

	ofxQuadCurveWarp warper;

	ofFbo subsectionScreen;
	ofRectangle sourceRect;
	float blendingWidthL, blendingWidthR;
};

class MultiScreenWarper {
public:
	MultiScreenWarper();
	~MultiScreenWarper();

	void setup(ofFbo *_screen, int _warperNum, int _divisionX, int _divisionY);
	void update();
	void draw();

	inline void setSourceScreen(ofFbo *_sourceScreen) {
		sourceScreen = _sourceScreen;
	}
	inline void setShowing(bool _showing) {
		if (_showing) {
			ui->enable();
			ui->setPosition(ofGetMouseX(), ofGetMouseY());
		}
		else {
			ui->disable();
			disableWarper();
		}
	}
	inline bool getShowing() {
		return ui->isVisible();
	}
	inline void toggleShowing() {
		setShowing(!getShowing());
	}

	ofxUISuperCanvas *ui;

private:
	ofFbo * sourceScreen;
	void drawCrossHatch();

	// blending
	int blendingWidth, oneWidth, oneHeight;
	float blendingGamma;
	ofFbo blendingGradiationFbo;
	void makeBlendingArea();
	void updateBlendingGamma();

	// warping
	int warperNum;
	vector<WarpingFbo *> warpingFbos;
	void recalcAllWarpingFbosSourceRect();
	string toggleWarper();
	void disableWarper();

	// ui
	int uiTheme;
	void uiEvent(ofxUIEventArgs &args);

	void save();
	void load();
};
