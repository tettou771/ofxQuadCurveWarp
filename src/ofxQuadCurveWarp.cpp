#include "ofxQuadCurveWarp.h"
#include "ofxQuadCurveWarpSettings.h"

ofxQuadCurveWarp::ofxQuadCurveWarp() {
	drawMode = TriangleMesh;
}

ofxQuadCurveWarp::~ofxQuadCurveWarp() {
}


void ofxQuadCurveWarp::setup(string _name) {
	name = _name;

	masterWarper.setup();
	sourceRect = ofRectangle(0, 0, 500, 500);
	auto targetRect = ofRectangle(100, 100, 500, 500);
	masterWarper.setSourceRect(sourceRect);
	masterWarper.setTargetRect(targetRect);

	// division num (grid num)
	divisionX = 16;
	divisionY = 16;

	divisionChanged();
	mouseHitSize = 5;
	nudgeValue = 0.25;

	editMode = NoEdit;
	flexibleEditTypeStr_hobar = "";
	flexibleEditTypeStr_select = "";

	minTightness = 1.0;
	maxTightness = 10.0;
	tightness = minTightness;
}

void ofxQuadCurveWarp::draw(ofTexture* _texture) {
	ofTexture* drawTexture = nullptr;
	if (_texture != nullptr) drawTexture = _texture;
	else if (texturePtr != nullptr) drawTexture = texturePtr;

	ofPushStyle();
	ofSetColor(255);
	if (drawTexture != nullptr) drawWithWarper(*drawTexture);

	switch (editMode) {
	case NoEdit:
		break;
	case MasterWarper:
		drawWarper(&masterWarper);
		drawCrossCursor();
		break;
	case Flexible:
	case Gaussian:
	case Point:
		drawLinkedWarpers();
		drawWarperVertexes();
		drawCrossCursor();
		break;
	}
	ofPopStyle();
}

void ofxQuadCurveWarp::setTexture(ofTexture* _texturePtr) {
	ofRectangle newSourceRect = ofRectangle(0, 0, _texturePtr->getWidth(), _texturePtr->getHeight());
	setTexture(_texturePtr, newSourceRect);
}

void ofxQuadCurveWarp::setTexture(ofTexture* _texturePtr, ofRectangle& _sourceRect) {
	texturePtr = _texturePtr;

	if (texturePtr == nullptr) return;

	// remake grid positions if sourceRect changed
	if (sourceRect != _sourceRect) {
		sourceRect = _sourceRect;
		sourceChanged();
	}
}

void ofxQuadCurveWarp::setDivision(int _divisionX, int _divisionY) {
	divisionX = _divisionX;
	divisionY = _divisionY;
	divisionChanged();
}

void ofxQuadCurveWarp::setSourceRect(ofRectangle& _sourceRect) {
	sourceRect.set(_sourceRect);
	sourceChanged();
}

void ofxQuadCurveWarp::setTargetRect(ofRectangle& _targetRect) {
	masterWarper.setTargetRect(_targetRect);
	divisionChanged();
}

void ofxQuadCurveWarp::save() {
	save("ofxQuadCurveWarpSettings_" + name + ".xml");
}

void ofxQuadCurveWarp::save(string path) {
	settings.getSettingsFrom(*this);
	settings.save(path);
}

void ofxQuadCurveWarp::load() {
	load("ofxQuadCurveWarpSettings_" + name + ".xml");
}

void ofxQuadCurveWarp::load(string path) {
	settings.load(path);

	// settingsをロードできていたら適用する
	if (settings.isLoaded()) {
		settings.applySettingsTo(*this);
	}
}

void ofxQuadCurveWarp::mouseKeyboardEnable() {
	if (mouseKeyboardEnabled) return;
	mouseKeyboardEnabled = true;

	ofRegisterKeyEvents(this);
	ofRegisterMouseEvents(this);

	if (editMode != MasterWarper) editModeChange(MasterWarper);
	else editModeChange(Flexible);
}

void ofxQuadCurveWarp::mouseKeyboardDisable() {
	if (!mouseKeyboardEnabled) return;
	mouseKeyboardEnabled = false;

	ofUnregisterKeyEvents(this);
	ofUnregisterMouseEvents(this);

	editModeChange(NoEdit);
}

void ofxQuadCurveWarp::setMouseKeyboardEnabled(bool enabled) {
	if (enabled) {
		mouseKeyboardEnable();
	}
	else {
		mouseKeyboardDisable();
	}
}

void ofxQuadCurveWarp::mouseKeyboardToggle() {
	setMouseKeyboardEnabled(!mouseKeyboardEnabled);
}

bool ofxQuadCurveWarp::getMouseKeyboardEnabled() {
	return mouseKeyboardEnabled;
}

void ofxQuadCurveWarp::keyPressed(ofKeyEventArgs& key) {

	if (key.key == 'e') toggleEditMode();
	else if (key.key == 'c') clearTargetOffset();

	// change selection
	else if (ofGetKeyPressed(OF_KEY_SHIFT) && key.key == OF_KEY_UP) selectMove(UP);
	else if (ofGetKeyPressed(OF_KEY_SHIFT) && key.key == OF_KEY_DOWN) selectMove(DOWN);
	else if (ofGetKeyPressed(OF_KEY_SHIFT) && key.key == OF_KEY_LEFT) selectMove(LEFT);
	else if (ofGetKeyPressed(OF_KEY_SHIFT) && key.key == OF_KEY_RIGHT) selectMove(RIGHT);

	// nudge selected edge
	else if (key.key == OF_KEY_UP) nudge(ofVec2f(0, -nudgeValue));
	else if (key.key == OF_KEY_DOWN) nudge(ofVec2f(0, nudgeValue));
	else if (key.key == OF_KEY_LEFT) nudge(ofVec2f(-nudgeValue, 0));
	else if (key.key == OF_KEY_RIGHT) nudge(ofVec2f(nudgeValue, 0));

	else if (key.key == OF_KEY_ESC) mouseKeyboardDisable();
}

void ofxQuadCurveWarp::keyReleased(ofKeyEventArgs& key) {
}

void ofxQuadCurveWarp::mouseMoved(ofMouseEventArgs& mouse) {
	flexibleEditTypeStr_hobar = "";

	// check mouse hits any points
	ofVec3f mousePos(mouse.x, mouse.y, 0);
	WarperVertex* hitW = nullptr;
	for (auto w : warperVertexes) {
		if (w->target.distanceSquared(mousePos) < mouseHitSize * mouseHitSize) {
			hitW = w;
			break;
		}
	}

	// if mouse hitted
	if (hitW != nullptr) {
		hoverWarperVertex(hitW);
	}

	// if mouse didn't hit
	else {
		hoveredWarperVertex = nullptr;
		for (auto w : warperVertexes) {
			w->highlightedLinearFactor = 1.0;
			w->highlightedNonLinearFactor = 0;
		}
	}
}

void ofxQuadCurveWarp::mouseDragged(ofMouseEventArgs& mouse) {
	ofVec3f move(mouse.x - ofGetPreviousMouseX(), mouse.y - ofGetPreviousMouseY(), 0);
	nudge(move);
}

void ofxQuadCurveWarp::mousePressed(ofMouseEventArgs& mouse) {

	// edit warper if left button pressed
	if (mouse.button == OF_MOUSE_BUTTON_LEFT) {
		// hoveredVertex as warperVertexes
		selectHoveredVertex();
	}

	// change edit mode if right button pressed
	else if (mouse.button == OF_MOUSE_BUTTON_RIGHT) {
		toggleEditMode();
	}

}

void ofxQuadCurveWarp::mouseReleased(ofMouseEventArgs& mouse) {
}

void ofxQuadCurveWarp::mouseScrolled(ofMouseEventArgs& mouse) {
	tightness += mouse.scrollY * 0.25;

	if (tightness < minTightness) tightness = minTightness;

	if (tightness > maxTightness) tightness = maxTightness;

	cout << tightness << endl;
}

void ofxQuadCurveWarp::hoverWarperVertex(WarperVertex* hovered) {
	hoveredWarperVertex = hovered;

	if (editMode == Flexible) {
		// 4 corners
		if ((hovered->sourceCoord.x == 0 || hovered->sourceCoord.x == 1.0)
			&&
			(hovered->sourceCoord.y == 0 || hovered->sourceCoord.y == 1.0)) {

			for (auto w : warperVertexes) {
				float distX = 1.0 - abs(hovered->sourceCoord.x - w->sourceCoord.x);
				float distY = 1.0 - abs(hovered->sourceCoord.y - w->sourceCoord.y);
				float sq = distX * distY;
				sq *= sq;

				w->highlightedNonLinearFactor = sq;
				w->highlightedLinearFactor = 1.0;
			}
			flexibleEditTypeStr_hobar = "Cornar";
		}

		// left and right edge
		else if (hovered->sourceCoord.x == 0 || hovered->sourceCoord.x == 1.0) {
			for (auto w : warperVertexes) {
				float wallDist;
				float sq;

				if (hovered->sourceCoord.y < w->sourceCoord.y) wallDist = 1.0 - hovered->sourceCoord.y;
				else wallDist = hovered->sourceCoord.y;
				if (wallDist == 0) sq = 0;
				else sq = abs(hovered->sourceCoord.y - w->sourceCoord.y) / wallDist;
				sq *= sq;

				float linear = 1.0 - abs(hovered->sourceCoord.x - w->sourceCoord.x);

				w->highlightedNonLinearFactor = 1.0 - sq;
				w->highlightedLinearFactor = linear;
			}
			flexibleEditTypeStr_hobar = "Edge";
		}

		// top and bottom edge
		else if (hovered->sourceCoord.y == 0 || hovered->sourceCoord.y == 1) {
			for (auto w : warperVertexes) {
				float wallDist;
				float sq;

				if (hovered->sourceCoord.x < w->sourceCoord.x) wallDist = 1.0 - hovered->sourceCoord.x;
				else wallDist = hovered->sourceCoord.x;
				if (wallDist == 0) sq = 0;
				else sq = abs(hovered->sourceCoord.x - w->sourceCoord.x) / wallDist;
				sq *= sq;

				float linear = 1.0 - abs(hovered->sourceCoord.y - w->sourceCoord.y);

				w->highlightedNonLinearFactor = 1.0 - sq;
				w->highlightedLinearFactor = linear;
			}
			flexibleEditTypeStr_hobar = "Edge";
		}

		// column selection mode
		else if (abs(hovered->sourceCoord.x - 0.5) < abs(hovered->sourceCoord.y - 0.5)) {
			for (auto f : hovered->rowFriends) {
				f->highlightedNonLinearFactor = 1.0;
				f->highlightedLinearFactor = 1.0;
				for (auto ff : f->columnFriends) {
					if (f == ff) continue; // 0除算を防ぐ
					float wallDist;
					if (f->sourceCoord.x < ff->sourceCoord.x) wallDist = 1.0 - f->sourceCoord.x;
					else wallDist = f->sourceCoord.x;
					float sq = (f->sourceCoord.x - ff->sourceCoord.x) / wallDist;
					sq *= sq;
					ff->highlightedNonLinearFactor = 1.0 - sq;
					ff->highlightedLinearFactor = 1.0;
				}
			}
			flexibleEditTypeStr_hobar = "Column";
		}

		// row selection mode
		else {
			for (auto f : hovered->columnFriends) {
				f->highlightedNonLinearFactor = 1.0;
				f->highlightedLinearFactor = 1.0;
				for (auto ff : f->rowFriends) {
					if (f == ff) continue; // 0除算を防ぐ
					float wallDist;
					if (f->sourceCoord.y < ff->sourceCoord.y) wallDist = 1.0 - f->sourceCoord.y;
					else wallDist = f->sourceCoord.y;
					float sq = (f->sourceCoord.y - ff->sourceCoord.y) / wallDist;
					sq *= sq;
					ff->highlightedNonLinearFactor = 1.0 - sq;
					ff->highlightedLinearFactor = 1.0;
				}
			}
			flexibleEditTypeStr_hobar = "Row";
		}
	}

	// gaussian selection mode
	else if (editMode == Gaussian) {
		// select with distance
		for (auto w : warperVertexes) {
			float distSq = hovered->sourceCoord.distanceSquared(w->sourceCoord);
			float sq = exp(-distSq * 6);

			w->highlightedNonLinearFactor = sq;
			w->highlightedLinearFactor = 1.0;
		}
	}

	// point selection mode (it just can select a point)
	else if (editMode == Point) {
		for (auto w : warperVertexes) {
			if (w == hovered) w->highlightedNonLinearFactor = 1.0;
			else w->highlightedNonLinearFactor = 0;
			w->highlightedLinearFactor = 1.0;
		}
	}
}

void ofxQuadCurveWarp::selectHoveredVertex() {
	selectedWarperVertex = hoveredWarperVertex;
	for (auto w : warperVertexes) {
		w->selectedNonLinearFactor = w->highlightedNonLinearFactor;
		w->selectedLinearFactor = w->highlightedLinearFactor;
		w->highlightedLinearFactor = 1.0;
		w->highlightedNonLinearFactor = 0;
	}

	flexibleEditTypeStr_select = flexibleEditTypeStr_hobar;
}

void ofxQuadCurveWarp::divisionChanged() {
	// clear all vertexes
	if (!warperVertexes.empty()) {
		for (auto w : warperVertexes) {
			delete w;
		}
		warperVertexes.clear();
	}
	if (!linkedWarpers.empty()) {
		for (auto l : linkedWarpers) {
			delete l;
		}
		linkedWarpers.clear();
	}

	// generate warperVertexes and miniWarpers
	float gridWidth = sourceRect.width / divisionX;
	float gridHeight = sourceRect.height / divisionY;
	for (int ix = 0; ix <= divisionX; ++ix) {
		for (int iy = 0; iy <= divisionY; ++iy) {
			WarperVertex* w = new WarperVertex();
			w->sourceCoord = ofVec3f((float)ix / divisionX, (float)iy / divisionY, 0);
			w->source = ofVec3f(ix * gridWidth, iy * gridHeight);
			warperVertexes.push_back(w);

			if (ix > 0 && iy > 0) {
				LinkedWarper* l = new LinkedWarper();
				l->warper.setup();
				ofRectangle miniSourceRect(w->source.x - gridWidth, w->source.y - gridHeight, gridWidth, gridHeight);
				l->warper.setSourceRect(miniSourceRect);
				l->linkVertex[1] = warperVertexes[(ix) * (divisionY + 1) + (iy - 1)];
				l->linkVertex[2] = warperVertexes[(ix) * (divisionY + 1) + (iy)];
				l->linkVertex[3] = warperVertexes[(ix - 1) * (divisionY + 1) + (iy)];
				l->linkVertex[0] = warperVertexes[(ix - 1) * (divisionY + 1) + (iy - 1)];
				l->warper.disableKeyboardShortcuts();
				l->warper.disableMouseControls();
				linkedWarpers.push_back(l);
			}
		}
	}

	// find warperVertex's friend
	// rowFriends : same row position
	// columnFriends: same column position
	for (int i = 0; i < warperVertexes.size(); ++i) {
		auto w = warperVertexes[i];

		// include self
		w->rowFriends.push_back(w);
		w->columnFriends.push_back(w);

		for (int j = i + 1; j < warperVertexes.size(); ++j) {
			auto f = warperVertexes[j];
			// same row
			if (w->sourceCoord.x == f->sourceCoord.x) {
				w->rowFriends.push_back(f);
				f->rowFriends.push_back(w);
			}
			// same column
			if (w->sourceCoord.y == f->sourceCoord.y) {
				w->columnFriends.push_back(f);
				f->columnFriends.push_back(w);
			}
		}
	}

	sourceChanged();
}

void ofxQuadCurveWarp::sourceChanged() {
	masterWarper.setSourceRect(sourceRect);

	// update warper vertex's source position
	for (auto w : warperVertexes) {
		w->source = ofVec3f(sourceRect.x + sourceRect.width * w->sourceCoord.x,
			sourceRect.y + sourceRect.height * w->sourceCoord.y, 0);
	}

	// update linked warper's source position
	for (auto l : linkedWarpers) {
		ofRectangle sourceRect;
		sourceRect.x = l->linkVertex[0]->source.x;
		sourceRect.y = l->linkVertex[0]->source.y;
		sourceRect.width = l->linkVertex[1]->source.x - l->linkVertex[0]->source.x;
		sourceRect.height = l->linkVertex[3]->source.y - l->linkVertex[0]->source.y;
		l->warper.setSourceRect(sourceRect);
	}

	masterWarperChanged();
}

void ofxQuadCurveWarp::masterWarperChanged() {
	// calc targetMaster with masterWarper(ofxQuadWarp)
	auto matrix = masterWarper.getMatrix();
	for (auto w : warperVertexes) {
		w->targetMaster = matrix.preMult(w->source);
	}

	targetOffsetChanged();
}

void ofxQuadCurveWarp::targetOffsetChanged() {
	// calc targets with targetOffsets
	for (auto v : warperVertexes) {
		v->target = v->targetMaster + v->targetOffset;
	}

	// update linkedWarpers
	for (auto l : linkedWarpers) {
		for (int i = 0; i < 4; ++i) {
			l->warper.setCorner(l->linkVertex[i]->target, i);
		}
		l->warperMatrix = l->warper.getMatrix();
	}

	// update mesh
	mesh.clear();
	for (auto l : linkedWarpers) {
		ofPoint* s = l->warper.getSourcePoints();
		ofPoint* t = l->warper.getTargetPoints();

		for (int i : {0, 1, 2, 2, 3, 0}) {
			mesh.addVertex(t[i]);
			mesh.addTexCoord(ofVec2f(s[i].x, s[i].y));
		}
	}
}

void ofxQuadCurveWarp::clearTargetOffset() {
	for (auto w : warperVertexes) {
		w->targetOffset = ofVec3f();
	}
	targetOffsetChanged();
}

void ofxQuadCurveWarp::toggleEditMode() {
	int next = (int)editMode + 1;
	if (next == (int)EditModeNum) next = 1;

	editModeChange(EditMode(next));
}

void ofxQuadCurveWarp::editModeChange(EditMode next) {
	editMode = (EditMode)next;

	// turn on/off masterWarper
	if (editMode == MasterWarper) masterWarper.show();
	else masterWarper.hide();

	// disselect warperVertexes
	for (auto w : warperVertexes) {
		w->highlightedLinearFactor = 1.0;
		w->highlightedNonLinearFactor = 0;
		w->selectedLinearFactor = 1.0;
		w->selectedNonLinearFactor = 0;
	}
	selectedWarperVertex = nullptr;
}

void ofxQuadCurveWarp::selectMove(Direction direction) {
	switch (editMode) {
	case MasterWarper:
		break;
	case Flexible:
	case Gaussian:
	case Point:
		if (selectedWarperVertex != nullptr) {
			switch (direction) {
			case UP:
				for (int i = selectedWarperVertex->rowFriends.size() - 1; i >= 0; --i) {
					auto &cf = selectedWarperVertex->rowFriends[i];
					if (cf->sourceCoord.y < selectedWarperVertex->sourceCoord.y) {
						hoverWarperVertex(cf);
						selectHoveredVertex();
						break;
					}
				}
				break;
			case DOWN:
				for (int i = 0; i < selectedWarperVertex->rowFriends.size(); ++i) {
					auto& cf = selectedWarperVertex->rowFriends[i];
					if (cf->sourceCoord.y > selectedWarperVertex->sourceCoord.y) {
						hoverWarperVertex(cf);
						selectHoveredVertex();
						break;
					}
				}
				break;
			case LEFT:
				for (int i = selectedWarperVertex->columnFriends.size() - 1; i >= 0; --i) {
					auto& cf = selectedWarperVertex->columnFriends[i];
					if (cf->sourceCoord.x < selectedWarperVertex->sourceCoord.x) {
						hoverWarperVertex(cf);
						selectHoveredVertex();
						break;
					}
				}
				break;
			case RIGHT:
				for (auto& cf : selectedWarperVertex->columnFriends) {
					if (cf->sourceCoord.x > selectedWarperVertex->sourceCoord.x) {
						hoverWarperVertex(cf);
						selectHoveredVertex();
						break;
					}
				}
				break;
			}
		}
		break;
	}
}

void ofxQuadCurveWarp::nudge(ofVec2f move) {
	switch (editMode) {
	case MasterWarper:
		masterWarperChanged();
		break;
	case Flexible:
	case Gaussian:
	case Point:
		for (auto w : warperVertexes) {
			w->targetOffset += move * w->getSelectedFactor(tightness);
		}
		targetOffsetChanged();
		break;
	}
}

void ofxQuadCurveWarp::drawWarper(ofxQuadWarp* w) {
	ofSetColor(0, 255, 0);
	w->drawQuadOutline();
	ofFill();
	w->drawCorners();
	ofSetColor(0, 255, 0);
	w->drawHighlightedCorner(); // mouse hover
	ofSetColor(255, 255, 0);
	w->drawSelectedCorner();
}

void ofxQuadCurveWarp::drawWarperVertexes() {
	ofFill();
	ofSetLineWidth(1);
	ofSetColor(0, 255, 0);
	float r = 3;

	ofRectMode(OF_RECTMODE_CENTER);

	// draw void rectangle if hovered selected
	ofNoFill();
	float maxSize = mouseHitSize * 2;
	for (auto w : warperVertexes) {
		float highlighted = w->getHighlightedFactor(tightness);
		if (highlighted == 0) continue;
		float size = highlighted * maxSize;
		ofDrawRectangle(w->target.x - size / 2, w->target.y - size / 2, size, size);
	}

	// draw solid rectangle if selected selected
	ofFill();
	for (auto w : warperVertexes) {
		float selected = w->getSelectedFactor(tightness);
		if (selected == 0) continue;
		float size = selected * maxSize;
		ofDrawRectangle(w->target.x - size / 2, w->target.y - size / 2, size, size);
	}

	// selected center vertex
	if (selectedWarperVertex != nullptr) {
		float size = maxSize * 1.5;
		auto w = selectedWarperVertex;
		ofNoFill();
		ofDrawRectangle(w->target.x - size / 2, w->target.y - size / 2, size, size);
	}

	ofRectMode(OF_RECTMODE_CORNER);
}

void ofxQuadCurveWarp::drawLinkedWarpers() {
	ofSetLineWidth(1);
	ofSetColor(0, 255, 0);
	for (auto l : linkedWarpers) {
		l->warper.drawQuadOutline();
	}
}

void ofxQuadCurveWarp::drawWithWarper(ofFbo& fbo) {
	if (texturePtr != nullptr && texturePtr->isAllocated()) {
		drawWithWarper(fbo.getTexture());
	}
}

void ofxQuadCurveWarp::drawWithWarper(ofTexture& texture) {
	if (!texture.isAllocated()) return;

	if (drawMode == TriangleMesh) {
		texture.bind();
		mesh.drawFaces();
		texture.unbind();
	}
	else if (drawMode == MicroWarper) {
		// using drawSubsection() in for loop is heavy (many overhead) because it use bind()
		// binding outside of for loop is ligit.

		texture.bind();

		for (auto l : linkedWarpers) {
			ofPushMatrix();
			ofMultMatrix(l->warperMatrix);
			ofPoint* p = l->warper.getSourcePoints();
			float x = p[0].x;
			float y = p[0].y;
			float z = 0;
			float w = p[1].x - p[0].x;
			float h = p[3].y - p[0].y;
			float sx = p[0].x + sourceRect.x;
			float sy = p[0].y + sourceRect.y;
			float sw = w;
			float sh = h;

			ofMesh subsectionMesh = texture.getMeshForSubsection(x, y, z, w, h, sx, sy, sw, sh, ofIsVFlipped(), OF_RECTMODE_CORNER);
			subsectionMesh.drawFaces();

			ofPopMatrix();
		}

		texture.unbind();
	}
}

void ofxQuadCurveWarp::drawCrossCursor() {
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	ofPushMatrix();
	ofTranslate(ofGetMouseX(), ofGetMouseY());

	// cursor
	ofSetLineWidth(2);
	ofSetColor(ofColor::white);
	int size = 10;
	ofDrawLine(-size, 0, size, 0);
	ofDrawLine(0, 0 - size, 0, size);

	// description text
	string description = "Edit\n  ";
	ofVec2f descriptionPos(10, 20);
	switch (editMode) {
	case MasterWarper:
		description += "Master warper";
		break;

	case Flexible:
		description += "Flexible ";
		if (flexibleEditTypeStr_select != "") description += flexibleEditTypeStr_select;
		else description += flexibleEditTypeStr_hobar;
		break;

	case Gaussian:
		description += "Gaussian";
		break;

	case Point:
		description += "Point";
		break;
	}
	ofDrawBitmapStringHighlight(description, descriptionPos, ofColor(0, 180), ofColor::white);

	// tightness
	switch (editMode) {
	case Flexible:
	case Gaussian:
		float barLength = 80;
		float barFillLength = ofMap(tightness, minTightness, maxTightness, barLength, 2);
		float barHeight = 12;

		ofNoFill();
		ofDrawRectangle(descriptionPos.x, descriptionPos.y + 20, barLength, barHeight);
		ofFill();
		ofDrawRectangle(descriptionPos.x + (barLength - barFillLength) / 2, descriptionPos.y + 20, barFillLength, barHeight);
	}


	ofPopMatrix();
	ofPopStyle();
}

