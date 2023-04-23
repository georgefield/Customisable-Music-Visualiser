#include "AudioManager.h"
#include "VisualiserShaderManager.h"
#include <functional>
#include <Vengine/IOManager.h>
#include <assert.h>
#include "SignalProcessingManager.h"
#include "SPvars.h"
#include "UIglobalFeatures.h"

#include "imgui.h"

miniaudio_api AudioManager::miniaudio;
std::string AudioManager::_audioFileName = "";

int AudioManager::_currentSampleExtraPrecisionTimerId;

int AudioManager::_stickySample = -1;

PlaybackInfo* AudioManager::_currentPlaybackInfo;
PlaybackInfo AudioManager::_loadedAudioInfo;
PlaybackInfo AudioManager::_loopbackInfo; 

bool AudioManager::_isUsingLoopback = false;
int AudioManager::_loopbackTimerId;
int AudioManager::_totalNewSamples = 0;
int AudioManager::_historyStartPosition = 0;
std::string AudioManager::_loopbackDeviceName = "";

float AudioManager::_volume = 1.0f;
int AudioManager::_lagTimerId;
bool AudioManager::_lagDetected = false;


void AudioManager::init()
{
	miniaudio.init();

	_currentPlaybackInfo = &_loadedAudioInfo;

	//PI = playback info
	initLoopbackPI();
	initLoadedAudioPI();

	setUseLoopback(false); //default using loaded audio

	Vengine::MyTiming::createTimer(_lagTimerId);
	Vengine::MyTiming::createTimer(_currentSampleExtraPrecisionTimerId);
	Vengine::MyTiming::createTimer(_loopbackTimerId);
}

void AudioManager::initLoopbackPI() {
	_loopbackInfo.isAudioLoaded = true;
	_loopbackInfo.isAudioPlaying = true;
	_loopbackInfo.sampleDataArrayPtr = new float[SP::consts._finalLoopbackStorageSize];
	_loopbackInfo.sampleDataArrayLength = SP::consts._finalLoopbackStorageSize;
	_loopbackInfo.sampleCounter = 0;
	_loopbackInfo.sampleDataArrayStartPosition = 0;
	_loopbackInfo.sampleRate = -1;
}

const std::string STARTUP_MUSIC_FILEPATH = "Resources/Audio/metronome.wav";
void AudioManager::initLoadedAudioPI() {
	load(STARTUP_MUSIC_FILEPATH); //program works by always having a song loaded
}

void AudioManager::resetLoopback()
{
	_loopbackInfo.nextCalculationSample = 0;
	_totalNewSamples = 0;
	Vengine::MyTiming::resetTimer(_loopbackTimerId);
	Vengine::MyTiming::startTimer(_loopbackTimerId);
}

std::string AudioManager::getLoopbackDeviceName()
{
	return _loopbackDeviceName;
}

void AudioManager::updateCurrentAudioInfo() {

	//update sample counter--
	if (isUsingLoopback()) {
		_loopbackInfo.sampleCounter = _loopbackInfo.sampleRate * Vengine::MyTiming::readTimer(_loopbackTimerId);
	}
	else {
		updateSampleCounterOfLoadedAudio();
	}
	//--

	//update nextcalculationsample and then decide whether to do signal processing--
	if (_currentPlaybackInfo->doSignalProcessing) //if did signal processing last frame
		_currentPlaybackInfo->nextCalculationSample += _currentPlaybackInfo->sampleRate / SP::vars._desiredCPS; //increment calculation sample

	//will only do calculation if sample counter bigger than calculation sample and will wait for loopback samples to catch up if not
	_currentPlaybackInfo->doSignalProcessing = (_currentPlaybackInfo->nextCalculationSample <= _currentPlaybackInfo->sampleCounter);

	if (!_currentPlaybackInfo->doSignalProcessing) {
		Vengine::MyTiming::resetTimer(_lagTimerId);
		return;
	}
	//--

	if (isUsingLoopback()) {
		int numNewSamples = 0;
		History<float>* loopbackDataPtr = miniaudio.getLoopbackDataHistoryPtr(numNewSamples);

		_totalNewSamples += numNewSamples;

		//total new samples is the offset of the array from the very first sample (because using history), so we take next calculation sample and remove the offset
		_historyStartPosition = _totalNewSamples - _loopbackInfo.nextCalculationSample + SP::consts._loopbackCacheSafetyBuffer;
		
		//if goes too out of phase set to a bunch of 0s
		if (_historyStartPosition < 0) {
			Vengine::warning("Sample gathering offset below 0 (not enough new samples)");
			//add exact amount of 0s to set head back to 0 (+ loopback safety buffer)
			for (int i = _totalNewSamples - _loopbackInfo.nextCalculationSample; i < 0; i++) {
				loopbackDataPtr->add(0);
			}
			_totalNewSamples += _loopbackInfo.nextCalculationSample - _totalNewSamples;

			_historyStartPosition = SP::consts._loopbackCacheSafetyBuffer; //reevaluate start position
		}

		if (_historyStartPosition >= SP::consts._requiredLoopbackCacheLength - SP::consts._loopbackCacheSafetyBuffer) {
			Vengine::warning("sample gathering offset too high (calculation sample not kept up with new samples)");
			resetLoopback();
			SignalProcessingManager::reset();

			//2 resets in less than 1 seconds then assume lagging and decrease CPS
			if (Vengine::MyTiming::readTimer(_lagTimerId) < SP::consts._lagTimeBeforeReducingCPS) {
				if (_lagDetected) {
					SP::vars._wasCPSautoDecreased = true;
					SP::vars._desiredCPS *= SP::consts._CPSreduceFactor; //decrease desired cps

					UIglobalFeatures::queueError("Cannot reach desired audio calculations per second (CPS), reducing CPS to " + std::to_string(SP::vars._desiredCPS)); //show error
				}
				else {
					_lagDetected = true;
					Vengine::MyTiming::startTimer(_lagTimerId);
				}
			}
			
			return;
		}
		else {
			//not lagging and been above 2 seconds
			if (Vengine::MyTiming::readTimer(_lagTimerId) >= SP::consts._lagTimeBeforeReducingCPS) {
				_lagDetected = false;
				Vengine::MyTiming::resetTimer(_lagTimerId);
			}
		}

		//get reverse and put it into sample data array
		for (int i = _historyStartPosition; i < _historyStartPosition + SP::consts._finalLoopbackStorageSize; i++) {
			_loopbackInfo.sampleDataArrayPtr[_historyStartPosition + SP::consts._finalLoopbackStorageSize - 1 - i] = _volume * miniaudio.getLoopbackDataHistoryPtr(numNewSamples)->get(i);
		}
	}
	else {
		_loadedAudioInfo.sampleDataArrayStartPosition = _loadedAudioInfo.nextCalculationSample;
		_loadedAudioInfo.isAudioLoadedThisFrame = false;

		//if next calculation sample behind for too long then assume lagging
		if (_loadedAudioInfo.nextCalculationSample < _loadedAudioInfo.sampleCounter) {
			Vengine::MyTiming::startTimer(_lagTimerId);
		}
		if (_loadedAudioInfo.nextCalculationSample > _loadedAudioInfo.sampleCounter) {
			Vengine::MyTiming::resetTimer(_lagTimerId);
		}

		if (Vengine::MyTiming::readTimer(_lagTimerId) > SP::consts._lagTimeBeforeReducingCPS) {
			SP::vars._desiredCPS *= SP::consts._CPSreduceFactor; //decrease desired cps
			UIglobalFeatures::queueError("Cannot reach desired audio calculations per second (CPS), reducing CPS to " + std::to_string(SP::vars._desiredCPS)); //show error

			SP::vars._wasCPSautoDecreased = true;
			Vengine::MyTiming::resetTimer(_lagTimerId);

			SignalProcessingManager::reset();
		}
	}
}

void AudioManager::setUseLoopback(bool useLoopback) {

	_isUsingLoopback = useLoopback;

	if (_isUsingLoopback) {
		pause();

		miniaudio.stopLoopback();
		miniaudio.startLoopback(_loopbackInfo.sampleRate, _loopbackDeviceName);
		_currentPlaybackInfo = &_loopbackInfo;

		audioInterruptOccured(0);
	}
	else {
		miniaudio.stopLoopback();
		_loopbackDeviceName = "Not using loopback";

		_currentPlaybackInfo = &_loadedAudioInfo;

		seekToSample(miniaudio.getCurrentSample());
	}

}

bool AudioManager::isUsingLoopback() {
	return _isUsingLoopback;
}


bool AudioManager::load(std::string filePath)
{
	if (!Vengine::IOManager::fileExists(filePath)) {
		Vengine::warning("File '" + filePath + "' does not exist");
		return false;
	}

	if (miniaudio.isLoaded()) {
		miniaudio.unloadAudio();
	}

	if (!miniaudio.loadAudio(filePath)) {
		miniaudio.unloadAudio();
		return false;
	}

	_audioFileName = filePath.substr(filePath.find_last_of("/") + 1);

	_loadedAudioInfo.isAudioLoadedThisFrame = true;
	_loadedAudioInfo.isAudioLoaded = true;
	_loadedAudioInfo.isAudioFinished = false;
	_loadedAudioInfo.isAudioPlaying = false;

	_loadedAudioInfo.sampleRate = miniaudio.getSampleRate();
	_loadedAudioInfo.sampleDataArrayLength = miniaudio.getAudioDataLength();
	_loadedAudioInfo.sampleDataArrayPtr = miniaudio.getAudioData();
	_loadedAudioInfo.sampleCounter = 0;

	_stickySample = -1;

	audioInterruptOccured(0);

	return true;
}

void AudioManager::play()
{
	if (isUsingLoopback()) {
		return;
	}

	_loadedAudioInfo.isAudioPlaying = true;
	if (_loadedAudioInfo.isAudioLoaded) {
		miniaudio.playAudio();
	}

	audioInterruptOccured(_loadedAudioInfo.sampleCounter);
}

void AudioManager::pause()
{
	_loadedAudioInfo.isAudioPlaying = false;
	if (_loadedAudioInfo.isAudioLoaded) {
		miniaudio.pauseAudio();
	}
	audioInterruptOccured(_loadedAudioInfo.sampleCounter);
}

void AudioManager::seekToSample(int sample) {

	if (_loadedAudioInfo.isAudioLoaded){
		//sanitise input
		if (sample < 0)
			sample = 0;
		if (sample >= _loadedAudioInfo.sampleDataArrayLength)
			sample = _loadedAudioInfo.sampleDataArrayLength - 1;

		miniaudio.seekToSample(sample);

		//reset current sample
		_loadedAudioInfo.sampleCounter = sample;
		_stickySample = -1;

		audioInterruptOccured(sample);
	}
}

void AudioManager::updateSampleCounterOfLoadedAudio()
{
	if (_loadedAudioInfo.isAudioLoaded && _loadedAudioInfo.isAudioPlaying) {
			if (_stickySample != miniaudio.getCurrentSample()) { //if mini audio not stuck on same sample
				_stickySample = miniaudio.getCurrentSample(); //update sticky sample

				Vengine::MyTiming::resetTimer(_currentSampleExtraPrecisionTimerId); //start timer to get extra 
				Vengine::MyTiming::startTimer(_currentSampleExtraPrecisionTimerId);
			}

			//add extra samples for extra precision
			int extraSamples = Vengine::MyTiming::readTimer(_currentSampleExtraPrecisionTimerId) * _loadedAudioInfo.sampleRate;
			_loadedAudioInfo.sampleCounter = miniaudio.getCurrentSample() + extraSamples;
			
			//check not over num samples
			if (_loadedAudioInfo.sampleCounter > _loadedAudioInfo.sampleDataArrayLength) {
				_loadedAudioInfo.sampleCounter = _loadedAudioInfo.sampleDataArrayLength;
				_loadedAudioInfo.isAudioFinished = true;
				pause();
			}
	}
}

void AudioManager::setVolume(float volume)
{
	_volume = volume;
	assert(volume >= 0.0f && volume <= 1.0f);
	miniaudio.setVolume(volume);
}

void AudioManager::audioInterruptOccured(int currentSample)
{
	std::cout << "Interrupt, next calc. sample = " << currentSample << std::endl;

	_currentPlaybackInfo->nextCalculationSample = currentSample;
	_currentPlaybackInfo->sampleCounter = currentSample;

	if (isUsingLoopback()) {
		resetLoopback();
	}
}


std::string AudioManager::getAudioFileName() {
	if (!miniaudio.isLoaded()) {
		return "No file loaded";
	}
	return _audioFileName;
}

std::string AudioManager::getAudioTimeInfoStr(int overrideSample) {
	if (!miniaudio.isLoaded()) {
		return "n/a";
	}
	if (overrideSample == -1) {
		overrideSample = _loadedAudioInfo.sampleCounter;
	}

	std::string ret = "";
	float seconds = float(overrideSample) / float(miniaudio.getSampleRate());
	int minutes = floor(seconds / 60.0f);
	seconds = fmodf(seconds, 60.0f);

	std::string secondsStr = "";
	if (seconds < 10)
		secondsStr += "0" + std::to_string(seconds).substr(0, 3);
	else
		secondsStr += std::to_string(seconds).substr(0, 4);


	ret += std::to_string(minutes) + ":" + secondsStr;
	ret += "/";

	float totalSeconds = float(miniaudio.getAudioDataLength()) / float(miniaudio.getSampleRate());
	int totalMinutes = floor(totalSeconds / 60.0f);
	totalSeconds = fmodf(totalSeconds, 60.0f);

	std::string totalSecondsStr = "";
	if (totalSeconds < 10)
		totalSecondsStr += "0" + std::to_string(totalSeconds).substr(0, 1);
	else
		totalSecondsStr += std::to_string(totalSeconds).substr(0, 2);

	ret += std::to_string(totalMinutes) + ":" + totalSecondsStr;

	return ret;
}