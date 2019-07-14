#include "MultiScreenWarper.h"



MultiScreenWarper::MultiScreenWarper() {
}


MultiScreenWarper::~MultiScreenWarper() {
}

//void MultiScreenWarper::setup(ofEventArgs & args) {
void MultiScreenWarper::setup(ofFbo *_screen, int _warperNum, int _divisionX, int _divisionY) {
	if (_screen == nullptr) {
		ofLogError() << "MultiScreenWarper setup error. Setted screen is nullptr.";
		return;
	}
	if (_warperNum < 1) {
		ofLogError() << "MultiScreenWarper setup error. warperNum is not allow under 1 but this is " << ofToString(_warperNum) << ".";
		return;
	}

	setSourceScreen(_screen);
	warperNum = _warperNum;

	uiTheme = 43;
	blendingWidth = 200;
	blendingGamma = 2.2;

	ui = new ofxUISuperCanvas("MultiScreenWarper");
	ui->setWidth(300);
	ui->setPosition(220, 0);
	ui->addFPS();
	ui->addIntSlider("UI theme", 0, 44, &uiTheme);
	ui->addSpacer();
	ui->addIntSlider("blendingWidth", 0, 500, &blendingWidth);
	ui->addSlider("blendingGamma", 1.0 / 6.0, 6.0, &blendingGamma);
	ui->addLabelButton("toggleWarper", false);
	ui->addTextArea("warpingTarget", "none");
	ui->addLabelButton("resetCurve", false);
	ui->addLabelButton("resetCurveAll", false);
	ui->addSpacer();
	ui->addLabelButton("save", false);
	ui->addLabelButton("load", false);
	ui->autoSizeToFitWidgets();
	ofAddListener(ui->newGUIEvent, this, &MultiScreenWarper::uiEvent);

	for (int i = 0; i < warperNum; ++i) {
		string name = "Warper_" + ofToString(i);
		ofRectangle targetRect(100 + i * 500, 100, 400, 400);
		auto newWarpingFbo = new WarpingFbo(name, targetRect, _divisionX, _divisionY);
		warpingFbos.push_back(newWarpingFbo);
	}
	makeBlendingArea();

	load();
}

void MultiScreenWarper::update() {
	ofPushStyle();
	ofSetColor(255);

	// if warper is editing, draw grid
	bool anyWarperIsEditing = false;
	for (auto &w : warpingFbos) {
		if (w->warper.isEnableEdit()) {
			anyWarperIsEditing = true;
			break;
		}
	}
	if (anyWarperIsEditing) {
		drawCrossHatch();
	}

	// screens copy from source screen
	for (auto &w : warpingFbos) {
		w->updateSubsectionScreen(sourceScreen, &blendingGradiationFbo);
	}


	/*
	// left side
	screenL.begin();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	sourceScreen->getTexture().drawSubsection(0, 0, oneWidth, oneHeight, 0, 0, oneWidth, oneHeight);
	ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
	blendingArea.draw(oneWidth - blendingWidth, 0);
	screenL.end();

	// right side
	screenR.begin();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	sourceScreen->getTexture().drawSubsection(0, 0, oneWidth, oneHeight, sourceScreen->getWidth() - oneWidth, 0, oneWidth, oneHeight);
	ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
	blendingArea.draw(blendingWidth, 0, -blendingWidth, oneHeight);
	screenR.end();
	*/

	ofPopStyle();
}

void MultiScreenWarper::draw() {
	ofPushStyle();
	ofSetColor(255);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableAntiAliasing();
	for (auto &w : warpingFbos) {
		w->draw();
	}
	ofPopStyle();


	return;
	// debug
	int x = 0;
	for (auto &w : warpingFbos) {
		w->subsectionScreen.draw(x, 0);
		x += w->subsectionScreen.getWidth();
	}
}

void MultiScreenWarper::drawCrossHatch() {
	int w = sourceScreen->getWidth();
	int h = sourceScreen->getHeight();

	ofPushStyle();
	ofEnableAntiAliasing();
	ofSetCircleResolution(h / 6);
	sourceScreen->begin();
	ofSetColor(255);
	ofSetLineWidth(2);
	ofNoFill();

	// outer rectangle
	ofDrawRectangle(1, 1, w - 2, h - 2);
	// inside circle
	ofDrawCircle(w / 2, h / 2, MIN(w / 2 - 1, h / 2 - 1));
	// diagonal lines
	ofDrawLine(0, 0, w, h);
	ofDrawLine(0, h, w, 0);
	// grid lines
	int nX = 32, nY = 18;
	for (int i = 1; i < nX; ++i) {
		float x = i * w / nX;
		ofDrawLine(x, 0, x, h);
	}
	for (int i = 1; i < nY; ++i) {
		float y = i * h / nY;
		ofDrawLine(0, y, w, y);
	}

	sourceScreen->end();
	ofPopStyle();
}

void MultiScreenWarper::makeBlendingArea() {
	oneWidth = (sourceScreen->getWidth() + blendingWidth * (warperNum - 1)) / warperNum;
	oneHeight = sourceScreen->getHeight();

	blendingGradiationFbo.clear();
	blendingGradiationFbo.allocate(blendingWidth, oneHeight, GL_RGB);
	updateBlendingGamma();

	recalcAllWarpingFbosSourceRect();
}

void MultiScreenWarper::updateBlendingGamma() {
	ofPushStyle();
	ofSetLineWidth(1);
	blendingGradiationFbo.begin();

	for (int ix = 0; ix < blendingGradiationFbo.getWidth(); ++ix) {
		float t = 1.0 * ix / blendingGradiationFbo.getWidth();
		float brightness = 1.0 - pow(t, blendingGamma);
		ofSetColor(brightness * 255);
		ofDrawLine(ix, 0, ix, oneHeight);
	}

	blendingGradiationFbo.end();
	ofPopStyle();
}

void MultiScreenWarper::recalcAllWarpingFbosSourceRect() {
	vector<float> dividingPosX;
	for (int i = 0; i < warperNum - 1; ++i) {
		//float x = (i + 1) * (sourceScreen->getWidth() - blendingWidth) / warperNum + blendingWidth / 2;
		//float x = (i + 1) * oneWidth - (i + 1.0 / 2.0) * blendingWidth;
		float x = oneWidth - blendingWidth / 2 + (oneWidth - blendingWidth) * i;

		dividingPosX.push_back(x);
	}

	for (int i = 0; i < warperNum; ++i) {
		auto &w = warpingFbos[i];

		float blendingWidthL, blendingWidthR;
		if (0 < i) blendingWidthL = blendingWidth;
		else blendingWidthL = 0;
		if (i < warperNum - 1)  blendingWidthR = blendingWidth;
		else blendingWidthR = 0;
		w->setBlendingWidthL(blendingWidthL);
		w->setBlendingWidthR(blendingWidthR);

		ofRectangle rect;
		if (i == 0) rect.x = 0;
		else rect.x = dividingPosX[i - 1] - blendingWidth / 2;
		rect.y = 0;
		rect.width = oneWidth;
		rect.height = oneHeight;
		w->setSourceRect(rect);
	}
}

string MultiScreenWarper::toggleWarper() {
	for (int i = 0; i < warperNum; ++i) {
		if (warpingFbos[i]->warper.isEnableEdit()) {
			warpingFbos[i]->warper.mouseKeyboardDisable();

			if (i < warperNum - 1) {
				warpingFbos[i + 1]->warper.mouseKeyboardEnable();
				return ofToString(i + 1);
			}
			else {
				return "None";
			}
		}
	}

	warpingFbos[0]->warper.mouseKeyboardEnable();
	return ofToString(0);
}

void MultiScreenWarper::disableWarper() {
	for (auto &w : warpingFbos) {
		w->warper.mouseKeyboardDisable();
	}
	string targetName = "none";
	((ofxUITextArea *)(ui->getWidget("warpingTarget")))->setTextString(targetName);
}

void MultiScreenWarper::uiEvent(ofxUIEventArgs & args) {
	if (args.getName() == "UI theme") {
		ui->setTheme(uiTheme);
	}
	if (args.getName() == "blendingWidth") {
		makeBlendingArea();
	}
	else if (args.getName() == "blendingGamma") {
		updateBlendingGamma();
	}
	else if (args.getName() == "resetCurve") {
		for (auto &w : warpingFbos) {
			if (w->warper.isEnableEdit()) {
				w->warper.clearTargetOffset();
			}
		}
	}
	else if (args.getName() == "resetCurveAll") {
		for (auto &w : warpingFbos) {
			w->warper.clearTargetOffset();
		}
	}
	else if (args.getName() == "save" && args.getBool()) {
		save();
	}
	else if (args.getName() == "load" && args.getBool()) {
		load();
	}
	else if (args.getName() == "toggleWarper" && args.getBool()) {
		string targetName = toggleWarper();
		((ofxUITextArea *)(ui->getWidget("warpingTarget")))->setTextString("Warping " + targetName);
	}
}

void MultiScreenWarper::save() {
	ui->saveSettings("settings_warper_ui.xml");
	for (auto &w : warpingFbos) {
		w->warper.save();
	}
}

void MultiScreenWarper::load() {
	ui->loadSettings("settings_warper_ui.xml");
	for (auto &w : warpingFbos) {
		w->warper.load();
	}
}
