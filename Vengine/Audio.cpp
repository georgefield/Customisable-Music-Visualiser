#include "Audio.h"
#include "MyErrors.h"
#include "IOManager.h"

#include <iostream>

using namespace Vengine;

Audio::Audio() : 
	_normalisedWavBuf(nullptr), 
	_audioLoaded(false),
	_audioPlaying(false),
	_audioQueued(false),
	_sampleRate(-1),
	_audioTimerId(-1)
{
}

void Audio::loadWav(const std::string& filepath) {
	if (SDL_LoadWAV(filepath.c_str(), &_audioSpec, &_waveBuf, &_waveLength) == nullptr) {
		fatalError("Could not load WAV file " + filepath);
	}
	_audioLoaded = true;

	_soundID = SDL_OpenAudioDevice(nullptr, 0, &_audioSpec, nullptr, 0);
	if (_soundID == 0) {
		fatalError("Could not open audio device for sound");
	}

	_sampleRate = _audioSpec.freq; //pass back sample rate
	normaliseWav();
}

void Audio::queueLoadedWav() {

	if (_audioLoaded == false) {
		warning("Audio not loaded so cannot queue");
		return;
	}

	if (_audioTimerId != -1) {
		MyTiming::stopTimer(_audioTimerId);
	}

	if (SDL_QueueAudio(_soundID, _waveBuf, _waveLength) < 0) {
		std::cout << SDL_GetError() << std::endl;
		fatalError("Could not queue audio");
	}
	_audioQueued = true;
}

void Vengine::Audio::pause()
{
	if (_audioQueued == false) {
		warning("Audio not queued");
		return;
	}
	if (_audioPlaying == false) {
		warning("Queued already is not currently playing");
		return;
	}

	_audioPlaying = false;
	MyTiming::pauseTimer(_audioTimerId);
	SDL_PauseAudioDevice(_soundID, 1); //1 is pause
}

void Vengine::Audio::play()
{

	if (_audioQueued == false) {
		warning("Audio not queued");
		return;
	}
	if (_audioPlaying == true) {
		warning("Audio already playing");
		return;
	}

	if (_audioTimerId == -1) { //start timer if first time play called
		MyTiming::startTimer(_audioTimerId);
	}
	else {
		MyTiming::unpauseTimer(_audioTimerId);
	}

	_audioPlaying = true;
	SDL_PauseAudioDevice(_soundID, 0); //0 is play
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

float* Audio::getNormalisedWavData() {
	if (_normalisedWavBuf != nullptr) {
		return _normalisedWavBuf;
	}
	Vengine::fatalError("No .wav file in buffer in Audio class when GetNormalisedWavData called");
	return nullptr;
}

int Vengine::Audio::getCurrentSample()
{
	if (_audioQueued == false) {
		Vengine::warning("No audio queued, asking for current sample invalid");
		return -1;
	}
	if (_audioTimerId == -1) {
		return 0;
	}

	return int(MyTiming::readTimer(_audioTimerId) * float(_sampleRate));
}
