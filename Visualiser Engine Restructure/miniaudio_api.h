#pragma once
#include <string>
class miniaudio_api
{
public:
	miniaudio_api();
	~miniaudio_api();

	void init();
	bool loadAudio(const std::string& filePath); //if returns false, call unload audio
	void unloadAudio();

	//functions that can be called once audio loaded--
	void playAudio();
	void pauseAudio();

	void seekToSample(int sample);

	void setVolume(float volume = 1.0f);

	bool isPlaying();
	bool isFinished();
	bool isLoaded();
	int getCurrentSample();
	float* getAudioData();
	int getAudioDataLength();
	int getSampleRate();
	//--

	void getDevices();
private:
	std::string _filePath;
	bool _initialised;
	
	//load data, fileLoaded => all others true
	bool _fileLoaded;
	bool _soundInitialised;
	bool _decoderInitialised;
	bool _memoryAllocated;

	float* _normalisedAudioData;
	int _audioDataLength;

	bool initEngine();
	bool initDecoderFromFile();
	bool initSoundFromFile();
};

