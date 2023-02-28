#pragma once
#include <Vengine/Audio.h>
class AudioManager
{
public:
	static void createUniformSetterFunctions(); //call on program start

	static void loadAudio(std::string filePath);
	static void play();
	static void pause() { _audio.pause(); }
	static void restart();

	static float* getAudioData() { return _audio.getNormalisedWavData(); }
	static int getCurrentSample() { return std::min(Uint32(_audio.getCurrentSample()), _audio.getWavLength()); }
	static int getSampleRate() { return _audio.getSampleRate(); }
	static std::string getAudioFilepath() { return _audioFilepath; }

	static bool isAudioLoaded() { return _audio.isAudioLoaded() && _audio.isAudioQueued(); }
	static bool isAudioFinished() { return _audio.getCurrentSample() >= int(_audio.getWavLength()); }
	static bool isAudioPlaying() { return _audio.isAudioPlaying() && !isAudioFinished(); }
private:
	static Vengine::Audio _audio;
	static std::string _audioFilepath;
};

