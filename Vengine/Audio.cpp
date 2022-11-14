#include "Audio.h"
#include "Errors.h"

#include <iostream>

using namespace Vengine;

Audio::Audio() {

}

void Audio::loadWav(const std::string& filepath) {
	if (SDL_LoadWAV(filepath.c_str(), &_audioSpec, &_waveBuf, &_waveLength) == nullptr) {
		fatalError("Could not load WAV file " + filepath);
	}

	_soundID = SDL_OpenAudioDevice(nullptr, 0, &_audioSpec, nullptr, 0);
	if (_soundID == 0) {
		fatalError("Could not open audio device for sound");
	}
}

void Audio::playSound() {

	if (SDL_QueueAudio(_soundID, _waveBuf, _waveLength) < 0) {
		std::cout << SDL_GetError() << std::endl;
		fatalError("Could not queue audio");
	}
	SDL_PauseAudioDevice(_soundID, 0); //0 is unpause aka play
}

Audio::~Audio() {
	//clear up data
	SDL_FreeWAV(_waveBuf);
	SDL_CloseAudioDevice(_soundID);
}