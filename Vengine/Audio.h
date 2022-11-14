#pragma once
#include <SDL/SDL.h>
#include <string>

namespace Vengine {

	class Audio
	{
	public:
		Audio();
		~Audio();
		void loadWav(const std::string& filepath);
		void playSound();
	private:
		SDL_AudioDeviceID _soundID;

		SDL_AudioSpec _audioSpec;
		Uint8* _waveBuf;
		Uint32 _waveLength;
	};

}