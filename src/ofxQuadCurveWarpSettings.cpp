#include "ofxQuadCurveWarp.h"
#include "ofxQuadCurveWarpSettings.h"

ofxQuadCurveWarpSettings::ofxQuadCurveWarpSettings() {
	loaded = false;
}

ofxQuadCurveWarpSettings::~ofxQuadCurveWarpSettings() {
}

void ofxQuadCurveWarpSettings::getSettingsFrom(ofxQuadCurveWarp & warper) {
	// ï™äÑêîÇï€ë∂
	divisionX = warper.divisionX;
	divisionY = warper.divisionY;

	// ï`âÊå≥ÇÃofRectangleÇï€ë∂
	sourceRect.set(warper.sourceRect);

	// masterWarperÇÃ4Ç¬äpÇï€ë∂
	auto points = warper.masterWarper.getTargetPoints();
	masterWarperTargetPoints = vector<ofPoint>();
	for (int i = 0; i < 4; ++i) {
		masterWarperTargetPoints.push_back(points[i]);
	}

	// offsetsÇï€ë∂
	targetOffsets = vector<ofVec2f>();
	targetOffsets.reserve((divisionX + 1) * (divisionY + 1));
	for (auto &w : warper.warperVertexes) {
		targetOffsets.push_back(w->targetOffset);
	}

	loaded = true;
}

void ofxQuadCurveWarpSettings::applySettingsTo(ofxQuadCurveWarp & warper) {
	warper.divisionX = divisionX;
	warper.divisionY = divisionY;
	warper.sourceRect = sourceRect;
	warper.masterWarper.setTargetPoints(masterWarperTargetPoints);
	warper.divisionChanged();
	for (int i = 0; i < targetOffsets.size(); ++i) {
		warper.warperVertexes[i]->targetOffset.set(targetOffsets[i]);
	}
	warper.targetOffsetChanged();
}

void ofxQuadCurveWarpSettings::save(string fileName) {
	ofXml xml;

	auto ofxQuadCurveWarpTag = xml.appendChild("ofxQuadCurveWarp");

	auto sourceRectTag = ofxQuadCurveWarpTag.appendChild("sourceRect");
	sourceRectTag.setAttribute("x", ofToString(sourceRect.x));
	sourceRectTag.setAttribute("y", ofToString(sourceRect.y));
	sourceRectTag.setAttribute("width", ofToString(sourceRect.width));
	sourceRectTag.setAttribute("height", ofToString(sourceRect.height));

	auto divisionTag = ofxQuadCurveWarpTag.appendChild("division");
	divisionTag.setAttribute("x", ofToString(divisionX));
	divisionTag.setAttribute("y", ofToString(divisionY));

	auto masterWarperTargetPointsTag = ofxQuadCurveWarpTag.appendChild("masterWarperTargetPoints");
	for (int i = 0; i < masterWarperTargetPoints.size(); ++i) {
		auto posTag = masterWarperTargetPointsTag.appendChild("pos");
		posTag.setAttribute("x", masterWarperTargetPoints[i].x);
		posTag.setAttribute("y", masterWarperTargetPoints[i].y);
	}

	auto targetOffsetsTag = ofxQuadCurveWarpTag.appendChild("targetOffsets");
	for (int i = 0; i < targetOffsets.size(); ++i) {
		auto posTag = targetOffsetsTag.appendChild("pos");
		posTag.setAttribute("x", targetOffsets[i].x);
		posTag.setAttribute("y", targetOffsets[i].y);
	}

	xml.save(fileName);
}

void ofxQuadCurveWarpSettings::load(string fileName) {
	ofXml xml;
	if (xml.load(fileName)) {
		loaded = true;

		auto ofxQuadCurveWarpTag = xml.getChild("ofxQuadCurveWarp");
		if (ofxQuadCurveWarpTag) {

			auto sourceRectTag = ofxQuadCurveWarpTag.getChild("sourceRect");
			if (sourceRectTag) {
				sourceRect.x = sourceRectTag.getAttribute("x").getFloatValue();
				sourceRect.y = sourceRectTag.getAttribute("y").getFloatValue();
				sourceRect.width = sourceRectTag.getAttribute("width").getFloatValue();
				sourceRect.height = sourceRectTag.getAttribute("height").getFloatValue();
			}

			auto divisionTag = ofxQuadCurveWarpTag.getChild("division");
			if (divisionTag) {
				divisionX = divisionTag.getAttribute("x").getFloatValue();
				divisionY = divisionTag.getAttribute("y").getFloatValue();
			}

			masterWarperTargetPoints.clear();
			auto masterWarperTargetPointsTag = ofxQuadCurveWarpTag.getChild("masterWarperTargetPoints");
			if (masterWarperTargetPointsTag) {
				auto posTags = masterWarperTargetPointsTag.getChildren("pos");
				for (auto &posTag : posTags) {
					ofPoint pos(
						posTag.getAttribute("x").getFloatValue(),
						posTag.getAttribute("y").getFloatValue());
					masterWarperTargetPoints.push_back(pos);
				}
			}

			auto targetOffsetsTag = ofxQuadCurveWarpTag.getChild("targetOffsets");
			if (targetOffsetsTag) {
				targetOffsets.clear();

				auto posTags = targetOffsetsTag.getChildren("pos");
				for (auto &posTag : posTags) {
					ofPoint pos(
						posTag.getAttribute("x").getFloatValue(),
						posTag.getAttribute("y").getFloatValue());
					targetOffsets.push_back(pos);
				}
			}
		}
	}
	else {
		ofLogNotice("\"" + fileName + "\" is not found.");
	}
}