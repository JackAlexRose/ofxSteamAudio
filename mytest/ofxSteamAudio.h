#pragma once

#include "ofMain.h"


class ofxSteamAudio {

public:
	ofxSteamAudio();
	virtual ~ofxSteamAudio();

	bool init();

	bool dispose();

	bool createAudioSource(const std::string filename);

	bool createListener();
};
