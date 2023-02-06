#pragma once
#include <SDL/SDL.h>
#include <string>

namespace Vengine {

	class Audio
	{
	public:
		Audio();
		~Audio();
		void loadWav(const std::string& filepath, int& _sampleRate);
		void playSound();
		void pauseSound();
		void unpauseSound();


		//getters
		float* getNormalisedWavData();
		Uint32 getWavLength() { return _waveLength; }

	private: 
		void normaliseWav();

		SDL_AudioDeviceID _soundID;

		SDL_AudioSpec _audioSpec;
		Uint8* _waveBuf;
		float* _normalisedWavBuf;
		Uint32 _waveLength;
	};

}