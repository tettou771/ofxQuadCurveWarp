#ifndef __ofxQuadCurveWarp_h__
#define __ofxQuadCurveWarp_h__

#include "ofMain.h"
#include "ofxQuadWarp.h"
#include "ofxQuadCurveWarpSettings.h"

class ofxQuadCurveWarpSettings;

class ofxQuadCurveWarp {
	friend ofxQuadCurveWarpSettings;

public:
	ofxQuadCurveWarp();
	~ofxQuadCurveWarp();

	void keyPressed(ofKeyEventArgs &key);
	void keyReleased(ofKeyEventArgs &key);
	void mouseMoved(ofMouseEventArgs &mouse);
	void mouseDragged(ofMouseEventArgs &mouse);
	void mousePressed(ofMouseEventArgs &mouse);
	void mouseReleased(ofMouseEventArgs &mouse);
	void mouseScrolled(ofMouseEventArgs &mouse);
	void mouseEntered(ofMouseEventArgs &mouse) {};
	void mouseExited(ofMouseEventArgs &mouse) {};

	void setup(string _name = "");
	void draw(ofTexture *_texture = nullptr);

	// set texture pointer that drawed with warper
	void setTexture(ofTexture *_texturePtr);
	void setTexture(ofTexture *_texturePtr, ofRectangle & _sourceRect);

	// set division (warping grid num)
	void setDivision(int _divisionX, int _divisionY);
	int getDivisionX() { return divisionX; }
	int getDivisionY() { return divisionY; }

	// set source rectangle
	// it can trim from source texture
	void setSourceRect(ofRectangle & _sourceRect);

	// set target rectangle
	// it is based with ofxQuadWarp 
	void setTargetRect(ofRectangle & _targetRect);

	// clear target offset values
	// but, quad warper doesn't reset.
	void clearTargetOffset();

	// set selection tightness (it used in curve warping)
	void setTightness(float _tightness) {
		tightness = MAX(minTightness, MIN(maxTightness, _tightness));
	}
	float getTightness() {
		return tightness;
	}

	// save warping parametors
	void save();
	void save(string path);
	void load();
	void load(string path);
	bool isEnableEdit() {
		return editMode != NoEdit;
	}

	// tightness of selection
	float tightness, minTightness, maxTightness;

	// enable / diable control
	void mouseKeyboardEnable();
	void mouseKeyboardDisable();

	string getName() {
		return name;
	}

	enum DrawMode {
		TriangleMesh,
		MicroWarper
	};
	void setDrawMode(DrawMode _mode) { drawMode = _mode; }
	DrawMode getDrawMode() { return drawMode; }

	// kind of edit mode
	enum EditMode {
		NoEdit,
		MasterWarper, // masterWarper's edge (ofxQuadWarp)
		Flexible, // column, row, edge, cornar
		Gaussian, // gaussian selection
		Point, // just a point selection
		EditModeNum
	};
	EditMode getEditmode() { return editMode; }

private:
	// warper name
	// save() and load() uses it. filename is
	// "ofxQuadCurveWarpSettings_" + name + ".xml"
	string name;

	// texture that to draw
	ofTexture * texturePtr;

	// base warper
	ofxQuadWarp masterWarper;

	// source image rectangle
	ofRectangle sourceRect;
	vector<ofxQuadWarp *> miniWarpers;

	// division of targetOffset (it's like grid num)
	int divisionX, divisionY;

	struct WarperVertex {
		ofVec3f sourceCoord; // position of source (canonical value 0.0 - 1.0)
		ofVec3f source; // position of source
		ofVec3f targetMaster; // it based masterWarper
		ofVec3f targetOffset; // offset from targetMaster
		ofVec3f target; // draw to that position（= targetMaster + targetOffset）

		vector<WarperVertex *> rowFriends; // same row vertex
		vector<WarperVertex *> columnFriends; // same column ertex

		// selected factor is select intensity.
		// when move it, move distance multiplied by this parametor
		// tightness effects NonLinear factor
		// tightness doesn't effect Linear factor
		// selected is clicked, they are ready to move
		// highlighted is mouse hover
		float selectedLinearFactor;
		float selectedNonLinearFactor;
		float highlightedLinearFactor;
		float highlightedNonLinearFactor;

		float getSelectedFactor(float tightness) {
			if (selectedNonLinearFactor == 0) return 0;
			else return selectedLinearFactor * (pow(selectedNonLinearFactor, tightness * tightness));
		}
		float getHighlightedFactor(float tightness) {
			if (highlightedNonLinearFactor == 0) return 0;
			else return highlightedLinearFactor * (pow(highlightedNonLinearFactor, tightness * tightness));
		}
	};
	vector<WarperVertex *> warperVertexes;
	ofMesh mesh; // drawing mesh used when TriangleMesh

	WarperVertex* hoveredWarperVertex = nullptr;
	WarperVertex* selectedWarperVertex = nullptr;
	void hoverWarperVertex(WarperVertex* hovered);
	void selectHoveredVertex();

	// miniWarper (warper cell)
	struct LinkedWarper {
		ofxQuadWarp warper;
		ofVec3f *linkVertex[4];
		ofMatrix4x4 warperMatrix;
	};
	vector<LinkedWarper *> linkedWarpers;

	void divisionChanged();
	void sourceChanged();
	void masterWarperChanged();
	void targetOffsetChanged();

	EditMode editMode;
	void toggleEditMode();
	void editModeChange(EditMode next);

	// hit collider size
	float mouseHitSize;

	// change selection in edit mode
	enum Direction {
		UP, DOWN, LEFT, RIGHT
	};
	void selectMove(Direction direction);

	// nudge value (cursor key move distance)
	float nudgeValue;
	void nudge(ofVec2f move);

	// draw UI
	void drawWarper(ofxQuadWarp *w);
	void drawWarperVertexes();
	void drawLinkedWarpers();

	void drawWithWarper(ofFbo &fbo);
	void drawWithWarper(ofTexture &texture);

	void drawCrossCursor();
	string flexibleEditTypeStr_hobar, flexibleEditTypeStr_select;

	// settings class for save and load
	ofxQuadCurveWarpSettings settings;

	DrawMode drawMode;
};

#endif