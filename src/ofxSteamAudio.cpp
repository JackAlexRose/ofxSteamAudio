#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>
#include "ofxSteamAudio.h"

bool ofxSteamAudio::init(int bufferSize, int sampleRate) {
	disconnect();

	iplCreateContext(nullptr, nullptr, nullptr, &_context);

	_iplRenderSettings.frameSize = _frameSize = bufferSize;
	_iplRenderSettings.samplingRate = _sampleRate = sampleRate;
	_iplRenderSettings.convolutionType = IPL_CONVOLUTIONTYPE_PHONON;

	IPLHrtfParams hrtfParams{ IPL_HRTFDATABASETYPE_DEFAULT, nullptr, nullptr };;

	_renderer = new IPLhandle;
	iplCreateBinauralRenderer(_context, _iplRenderSettings, hrtfParams, _renderer);

	IPLAudioFormat mono;
	_mono.channelLayoutType = IPL_CHANNELLAYOUTTYPE_SPEAKERS;
	_mono.channelLayout = IPL_CHANNELLAYOUT_MONO;
	_mono.numSpeakers = 1;
	_mono.channelOrder = IPL_CHANNELORDER_INTERLEAVED;

	IPLAudioFormat stereo;
	_stereo.channelLayoutType = IPL_CHANNELLAYOUTTYPE_SPEAKERS;
	_stereo.numSpeakers = 2;
	_stereo.channelLayout = IPL_CHANNELLAYOUT_STEREO;
	_stereo.channelOrder = IPL_CHANNELORDER_INTERLEAVED;

	_initialised = true;

	return true;
}

void ofxSteamAudio::disconnect() {

	_initialised = false;
	_listenerType = Uninitialised;

	for (int i = 0; i < _sources.size(); i++) {
		try {
			_sources[i].destroy();
		}
		catch (const std::exception &ex) {
			ofLog(OF_LOG_ERROR, "Disconnect threw an exception when attempting to destroy the audio source: " + _sources[i].soundName + ": " + (string)ex.what());
		}
	}

	try {
		if (_renderer != nullptr) {
			iplDestroyBinauralRenderer(_renderer);
		}
	}
	catch (const std::exception &ex) {
		ofLog(OF_LOG_ERROR, "Disconnect threw an exception when attempting to destroy the binaural renderer: " + (string)ex.what());
	}

	try {
		if (_context != nullptr) {
			iplDestroyContext(&_context);
		}
	}
	catch (const std::exception &ex) {
		ofLog(OF_LOG_ERROR, "Disconnect threw an exception when attempting to destroy the context: " + (string)ex.what());
	}

	iplCleanup();
}

void ofxSteamAudio::addSoundSource(ofxSteamAudioSoundSource source) {
	source.effect = new IPLhandle;
	iplCreateBinauralEffect(*_renderer, _mono, _stereo, source.effect);

	_sources.push_back(source);
}

void ofxSteamAudio::createListenerAndAttachToNode(ofNode& parent) {
	_parent = &parent;
	_listenerType = Child;
}

void ofxSteamAudio::createListenerDiscrete() {
	_listenerType = Discrete;
}

void ofxSteamAudio::updateListenerPosition(ofNode listenerPosition) {
	_listener.direction.x = listenerPosition.getLookAtDir().x;
	_listener.direction.y = listenerPosition.getLookAtDir().y;
	_listener.direction.z = listenerPosition.getLookAtDir().z;

	_listener.location.x = listenerPosition.getGlobalPosition().x;
	_listener.location.y = listenerPosition.getGlobalPosition().y;
	_listener.location.z = listenerPosition.getGlobalPosition().z;

	_listener.up.x = listenerPosition.getUpDir().x;
	_listener.up.y = listenerPosition.getUpDir().y;
	_listener.up.z = listenerPosition.getUpDir().z;
}

void ofxSteamAudio::processAudio(ofSoundBuffer &output) {

	int numFrames = output.getNumFrames();
	int bufferSize = numFrames * output.getNumChannels();

	if (!_initialised) {
		ofLog() << "Steam Audio not initialised, run init function";
		return;
	}
	if (_processing) {
		return;
	}
	if (_listenerType == Uninitialised) {
		ofLog() << "Listener not created, run 'createListener' with a provided ofNode (can also be a camera or easyCam)";
		return;
	}
	else if (_listenerType == Child) {
		updateListenerPosition(*_parent);
	}

	_processing = true;

	_inBuffers.clear();
	_outBuffers.clear();

	IPLVector3 dir;
	dir.x = 10;
	dir.y = 0;
	dir.z = 0;

	std::vector<SpatialBuffer> outBuffersVector;
	
	for (int i = 0; i < _sources.size(); i++) { 
		//IPLAudioBuffer inputBuffer;
		SpatialBuffer* outputBuffer = new SpatialBuffer;

		inputBuffer.format = _mono;
		inputBuffer.numSamples = numFrames;
		inputBuffer.interleavedBuffer = new IPLfloat32[numFrames];
		
		outputBuffer->buffer.format = _stereo;
		outputBuffer->buffer.numSamples = bufferSize;
		outputBuffer->buffer.interleavedBuffer = new IPLfloat32[bufferSize];

		//Because this is a pointer, this might only work for one source (the last one). Check with multiple sources
		for (int t = 0; t < numFrames; ++t)
		{
			//inputBuffer.interleavedBuffer[t] = _sources[i].getSample(t + _playhead);
			//inputBuffer.interleavedBuffer[t * 2] = _sources[i].getSample(t);
			//inputBuffer.interleavedBuffer[t * 2 + 1] = _sources[i].getSample(t);
			

			_iplDataDry[t] = _sources[i].getSample(t + _playhead);
			//_iplDataDry[t * 2 + 1] = _sources[i].getSample(t);
		}
		_playhead += numFrames;
		this->inputBuffer.interleavedBuffer = _iplDataDry;

		IPLVector3 sourcePosition;

		sourcePosition.x = _sources[i].location.x;
		sourcePosition.y = _sources[i].location.y;
		sourcePosition.z = _sources[i].location.z;

		IPLVector3 direction = iplCalculateRelativeDirection(sourcePosition, _listener.location, _listener.direction, _listener.up);

		iplApplyBinauralEffect(*_sources[i].effect, *_renderer, inputBuffer, dir, IPL_HRTFINTERPOLATION_NEAREST, 1, outputBuffer->buffer);
		
		for (int b = 0; b < bufferSize; b++) {
			output[b] = this->inputBuffer.interleavedBuffer[b];
		}
		_processing = false;

		return;
		//_outBuffers.push_back(outputBuffer);
		outBuffersVector.push_back(*outputBuffer);
	}

	mixAudioSamples(output, bufferSize, outBuffersVector);
	_processing = false;
}

int ofxSteamAudio::getSampleRate() {
	return _sampleRate;
}

void ofxSteamAudio::setSampleRate(int sampleRate) {
	_sampleRate = sampleRate;
}

int ofxSteamAudio::getFrameSize() {
	return _frameSize;
}

void ofxSteamAudio::setFrameSize(int frameSize) {
	_frameSize = frameSize;
}

void ofxSteamAudio::mixAudioSamples(ofSoundBuffer &outputBuffer, int bufferSize, std::vector<SpatialBuffer> spatialisedSound) {
	float* mixed = new float[bufferSize];

	for (int i = 0; i < bufferSize; ++i) {
		for (int b = 0; b < _outBuffers.size(); ++b) {
			mixed[i] += (_outBuffers[b]->buffer.interleavedBuffer[i] * _sources[b].getGain());
		}
	}
	
	double RMS = 0;

	for (int i = 0; i < bufferSize; ++i) {
		RMS += mixed[i] * mixed[i];
	}

	RMS /= bufferSize;
	RMS = sqrt(RMS);

	double GainMaster = 0x7fff / (sqrt(2)*RMS);

	for (int i = 0; i < bufferSize; ++i) {
		mixed[i] *= GainMaster;

		if (mixed[i] > 0x7fff) {
			mixed[i] = 0x7fff;
		}
		else if (mixed[i] < -0x7fff) {
			mixed[i] = -0x7fff;
		}

		if (i % 2 == 0) {
			outputBuffer.getSample(i, 0) = mixed[i];
		}
		else {
			outputBuffer.getSample(i, 1) = mixed[i];
		}
	}
}