#pragma once
#include <SDL/SDL.h>
#include <string>
#include "MyTiming.h"

namespace Vengine {

	class Audio
	{
	public:
		Audio();
		~Audio();
		void loadWav(const std::string& filepath);
		void queueLoadedWav();
		void pause();
		void play();


		//getters
		float* getNormalisedWavData();
		int getCurrentSample();
		Uint32 getWavLength() { return _waveLength; }
		int getSampleRate() { return _sampleRate; }

		bool isAudioLoaded() { return _audioLoaded; }
		bool isAudioQueued() { return _audioQueued; }
		bool isAudioPlaying() { return _audioPlaying; }
		
	private: 
		int _sampleRate;
		int _audioTimerId;

		bool _audioLoaded;
		bool _audioQueued;
		bool _audioPlaying;

		void normaliseWav();

		SDL_AudioDeviceID _soundID;

		SDL_AudioSpec _audioSpec;
		Uint8* _waveBuf;
		float* _normalisedWavBuf;
		Uint32 _waveLength;
	};

}