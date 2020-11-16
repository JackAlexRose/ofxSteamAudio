#include "ofMain.h"
#include "phonon.h"
#include "ofxAudioFile.h"

class ofxSteamAudioSoundSource {

public:

	ofVec3f location;
	string soundName;
	IPLhandle* effect;

	int sampleStart;

	void setGain(float gain);
	float getGain();

	void load(const std::string filepath, string sourceName);
	void destroy();

	void play();

	float getSample(int position);

protected:
	ofxAudioFile _audioFile;

	float _gain = 1.0;

};