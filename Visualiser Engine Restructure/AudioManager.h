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

	static float* getSampleData();
	static int getNumSamples();
	static int getCurrentSample();
	static int getSampleRate();

	static bool isAudioPlaying();
	static bool isAudioFinished();
	static bool isAudioLoadedThisFrame();
	static bool isAudioLoaded();

	static void showImguiDebugWindow();

private:
	static miniaudio_api miniaudio;
	static std::string _audioFileName;

	static bool _audioLoadedThisFrame;

	static int _currentSampleExtraPrecisionTimerId; //for some reason mini audio current sample works in 
	static int _stickySample;
	static int _currentSample;

	static void createUniformSetterFunctions(); //call on program start
};

