#pragma once
#include "miniaudio_api.h"
#include <Vengine/MyTiming.h>

class AudioManager
{
public:
	static void init();

	static bool load(std::string filePath);
	static void play();
	static void pause();
	static void seekToSample(int sample);

	static float* getSampleData();
	static int getNumSamples();
	static int getCurrentSample();
	static int getSampleRate();
	static std::string getAudioFileName();
	static std::string getAudioTimeInfoStr(int overrideSample = -1);

	static bool isAudioPlaying();
	static bool isAudioFinished();
	static bool isAudioLoadedThisFrame();
	static bool isAudioLoaded();

	static void showImguiDebugWindow(); 

	static void setVolume(float volume);

private:
	static miniaudio_api miniaudio;
	static std::string _audioFileName;

	static bool _audioLoadedThisFrame;
	static float _volume;

	static int _currentSampleExtraPrecisionTimerId; //for some reason mini audio current sample works in 
	static int _stickySample;
	static int _currentSample;

	static void createUniformSetterFunctions(); //call on program start
};

