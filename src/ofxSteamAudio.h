#pragma once
#include "ofMain.h"
#include "ofxSteamAudioSoundSource.h"
#include "phonon.h"
#include <vector>

class ofxSteamAudio {

public:
	bool init(int frameSize, int sampleRate);

	void disconnect();

	void addSoundSource(ofxSteamAudioSoundSource source);

	void createListenerAndAttachToNode(ofNode& parent);
	void createListenerDiscrete();
	void updateListenerPosition(ofNode listenerPosition);

	int getSampleRate();
	int getFrameSize();
	void setSampleRate(int sampleRate);
	void setFrameSize(int frameSize);

	void processAudio(ofSoundBuffer &outputBuffer);

protected:
	int _frameSize;
	int _sampleRate;
	IPLAudioBuffer inputBuffer;
	int _playhead = 0;

	bool _initialised = false;
	bool _processing = false;

	struct SpatialBuffer {
		IPLAudioBuffer buffer;
		ofVec3f location;
	};

	std::vector<ofxSteamAudioSoundSource> _sources;
	std::vector<SpatialBuffer*> _inBuffers;
	std::vector<SpatialBuffer*> _outBuffers;
	IPLRenderingSettings _iplRenderSettings;
	IPLhandle _context;
	IPLhandle* _renderer;

	IPLfloat32 _iplDataDry[512];
	IPLfloat32 _iplDataSpatilized[1024];

	IPLAudioFormat _mono;
	IPLAudioFormat _stereo;

	struct Listener {
		IPLVector3 location;
		IPLVector3 direction;
		IPLVector3 up;
	};

	ofNode * _parent = nullptr;

	Listener _listener;
	enum ListenerType { Uninitialised, Child, Discrete }_listenerType = Uninitialised;

	void mixAudioSamples(ofSoundBuffer &outputBuffer, int bufferSize, std::vector<SpatialBuffer> spatialisedSound);
};

