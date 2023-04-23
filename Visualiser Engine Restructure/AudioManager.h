#pragma once
#include "miniaudio_api.h"
#include <Vengine/MyTiming.h>

struct PlaybackInfo {
	int sampleRate = -1;

	float* sampleDataArrayPtr = nullptr;
	int sampleDataArrayLength;
	int sampleDataArrayStartPosition;
	int sampleCounter; //continuous even in loopback mode, counts up at sample rate from loopback activation
	int nextCalculationSample;
	bool doSignalProcessing;

	bool isAudioPlaying = false;
	bool isAudioFinished = false;
	bool isAudioLoaded = false;
	bool isAudioLoadedThisFrame = false;
};

class AudioManager
{
public:
	static void init();
	static void updateCurrentAudioInfo(); //call every frame

	static PlaybackInfo* _currentPlaybackInfo;

	//playing from file--
	static bool load(std::string filePath);
	static void play();
	static void pause();
	static void seekToSample(int sample);

	static std::string getAudioFileName();
	static std::string getAudioTimeInfoStr(int overrideSample = -1);
	//--

	//using loopback--
	static void setUseLoopback(bool useLoopback);
	static bool isUsingLoopback();
	static std::string getLoopbackDeviceName();
	//--

	static void setVolume(float volume);
	static void audioInterruptOccured(int currentSample);

	//setter functions--
	static int getCurrentSample() { return _currentPlaybackInfo->sampleCounter; }
	static int getTotalAudioDataLength() { return _currentPlaybackInfo->sampleDataArrayLength; }
	static int getSampleRate() { return _currentPlaybackInfo->sampleRate; }
	//--
private:
	static PlaybackInfo _loopbackInfo;
	static PlaybackInfo _loadedAudioInfo;

	static miniaudio_api miniaudio;
	static std::string _audioFileName;

	static int _currentSampleExtraPrecisionTimerId; //for some reason mini audio current sample works in 
	static int _stickySample;

	static bool _isUsingLoopback;
	static int _loopbackTimerId;
	static int _totalNewSamples;
	static int _historyStartPosition;
	static std::string _loopbackDeviceName;

	static float _volume;
	static int _lagTimerId;
	static bool _lagDetected;

	static void updateSampleCounterOfLoadedAudio();

	static void resetLoopback();
	static void initLoopbackPI();
	static void initLoadedAudioPI();
};

