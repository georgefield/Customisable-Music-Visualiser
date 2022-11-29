#include "Audio.h"
#include "MyErrors.h"
#include "IOManager.h"

#include <iostream>

using namespace Vengine;

Audio::Audio() {

}

void Audio::loadWav(const std::string& filepath, int& sampleRate) {
	if (SDL_LoadWAV(filepath.c_str(), &_audioSpec, &_waveBuf, &_waveLength) == nullptr) {
		fatalError("Could not load WAV file " + filepath);
	}

	_soundID = SDL_OpenAudioDevice(nullptr, 0, &_audioSpec, nullptr, 0);
	if (_soundID == 0) {
		fatalError("Could not open audio device for sound");
	}

	sampleRate = _audioSpec.freq; //pass back sample rate
	normaliseWav();
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
	
	delete[] _normalisedWavBuf;
}

int byteArrToInt(Uint8* vec, int length) {
	if (length > 4 or length < 1) {
		fatalError("can only convert 4 bytes or less to integer");
	}
	int ret = 0;
	for (int i = 0; i < length; i++) {
		ret |= vec[i] << 8 * (length - i - 1);
	}
	return ret;
}

void flipEndian(Uint8* in, int length) {
	if (length > 4 or length < 1) {
		fatalError("can only flip endian of 4 bytes or less to integer");
	}
	unsigned char tmp;
	for (int i = 0; i < (length / 2); i++) {
		tmp = in[i];
		in[i] = in[length - i - 1];
		in[length - i - 1] = tmp;
	}
}

void Audio::normaliseWav() {

	_normalisedWavBuf = new float[_waveLength];
	
	//convert to signed int format single channel (easy to normalise)
	SDL_AudioCVT cvt;
	SDL_BuildAudioCVT(&cvt, _audioSpec.format, 2, _audioSpec.freq, AUDIO_S32MSB, 1, _audioSpec.freq);
	SDL_assert(cvt.needed); // obviously, this one is always needed.
	cvt.len = _waveLength;
	cvt.buf = (Uint8*)SDL_malloc(cvt.len * cvt.len_mult);
	SDL_memcpy(cvt.buf, _waveBuf, _waveLength);

	if (SDL_ConvertAudio(&cvt) < 0) {
		fatalError("Audio failed to convert to float type");
	}

	//convert to float between 0 & 1
	for (int i = 0; i < cvt.len * cvt.len_mult; i+=4) {
		int ret = byteArrToInt(&(cvt.buf[i]), 4);
		_normalisedWavBuf[i/4] = (float(ret) / INT32_MAX);
	}
}