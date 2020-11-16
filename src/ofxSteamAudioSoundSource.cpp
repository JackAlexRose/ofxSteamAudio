#include "ofxSteamAudioSoundSource.h"

void ofxSteamAudioSoundSource::load(const std::string filepath, string sourceName) {
	_audioFile.setVerbose(true);
	ofSetLogLevel(OF_LOG_VERBOSE);

	if (ofFile::doesFileExist(filepath)) {
		_audioFile.load(filepath);
		if (!_audioFile.loaded()) {
			ofLogError() << "Error loading file, double check the file path";
			return;
		}
	}
	else {
		ofLogError() << "Input file does not exist";
		return;
	}

	soundName = sourceName;

	ofLog() << soundName << " Sound source loaded";
	ofLog() << _audioFile.channels() << " channels loaded";
}

void ofxSteamAudioSoundSource::play() {

}

void ofxSteamAudioSoundSource::destroy() {
	iplDestroyBinauralEffect(effect);
}

float ofxSteamAudioSoundSource::getSample(int position) {
	if (position > _audioFile.length())
		return 0;
	else return _audioFile.sample(position, 0);
}

void ofxSteamAudioSoundSource::setGain(float gain) {
	if (gain > 1.0 || gain < 0.0) {
		ofLogError() << "Gain value invalid. Must be between 0 and 1.0";
		return;
	}

	_gain = gain;
}

float ofxSteamAudioSoundSource::getGain() {
	return _gain;
}