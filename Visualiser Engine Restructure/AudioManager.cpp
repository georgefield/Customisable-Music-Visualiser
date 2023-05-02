#include "AudioManager.h"
#include "VisualiserShaderManager.h"
#include <functional>
#include <Vengine/IOManager.h>
#include <assert.h>
#include "SignalProcessingManager.h"
#include "VisVars.h"
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
int AudioManager::_sampleCounterOffset = 0;

float AudioManager::_volume = 1.0f;
int AudioManager::_loopbacklagTimerId;
int AudioManager::_numLagsInTimeWindow = 0;
int AudioManager::_fileAudioLagTimerId;


void AudioManager::init()
{
	miniaudio.init();

	_currentPlaybackInfo = &_loadedAudioInfo;

	//PI = playback info
	initLoopbackPI();
	initLoadedAudioPI();

	setUseLoopback(false); //default using loaded audio

	Vengine::MyTiming::createTimer(_loopbacklagTimerId);
	Vengine::MyTiming::createTimer(_fileAudioLagTimerId);
	Vengine::MyTiming::createTimer(_currentSampleExtraPrecisionTimerId);
	Vengine::MyTiming::createTimer(_loopbackTimerId);
}

void AudioManager::initLoopbackPI() {
	_loopbackInfo.isAudioLoaded = true;
	_loopbackInfo.isAudioPlaying = true;
	_loopbackInfo.sampleDataArrayPtr = new float[Vis::consts._finalLoopbackStorageSize];
	_loopbackInfo.sampleDataArrayLength = Vis::consts._finalLoopbackStorageSize;
	_loopbackInfo.sampleCounter = 0;
	_loopbackInfo.sampleDataArrayStartPosition = 0;
	_loopbackInfo.sampleRate = -1;
}

const std::string STARTUP_MUSIC_FILEPATH = "Resources/Audio/metronome.wav";
void AudioManager::initLoadedAudioPI() {
	load(STARTUP_MUSIC_FILEPATH); //program works by always having a song loaded
}

void AudioManager::resetLoopback(int sample)
{
	_loopbackInfo.nextCalculationSample = sample + _loopbackInfo.sampleRate / Vis::vars._desiredCPS;
	_totalNewSamples = sample;
	_sampleCounterOffset = sample - (_loopbackInfo.sampleRate * Vengine::MyTiming::readTimer(_loopbackTimerId));
}

std::string AudioManager::getLoopbackDeviceName()
{
	return _loopbackDeviceName;
}

void AudioManager::updateCurrentAudioInfo() {

	//update sample counter--
	if (isUsingLoopback()) {
		_loopbackInfo.sampleCounter = _loopbackInfo.sampleRate * Vengine::MyTiming::readTimer(_loopbackTimerId) + _sampleCounterOffset;

		//update nextcalculationsample and then decide whether to do signal processing--
		if (_loopbackInfo.doSignalProcessing) //if did signal processing last frame
			_loopbackInfo.nextCalculationSample += _loopbackInfo.sampleRate / Vis::vars._desiredCPS; //increment calculation sample

		//will only do calculation if sample counter bigger than calculation sample and will wait for loopback samples to catch up if not
		_loopbackInfo.doSignalProcessing = (_loopbackInfo.nextCalculationSample <= _loopbackInfo.sampleCounter);
		if (!_loopbackInfo.doSignalProcessing) {
			Vengine::MyTiming::resetTimer(_fileAudioLagTimerId);
			return;
		}
		//--

		int numNewSamples = 0;
		History<float>* loopbackDataPtr = miniaudio.getLoopbackDataHistoryPtr(numNewSamples);

		_totalNewSamples += numNewSamples;

		//total new samples is the offset of the array from the very first sample (because using history), so we take next calculation sample and remove the offset
		_historyStartPosition = _totalNewSamples - _loopbackInfo.nextCalculationSample + Vis::consts._loopbackCacheSafetyBuffer;

		//if goes too out of phase set to a bunch of 0s
		if (_historyStartPosition < 0) {
			Vengine::warning("Sample gathering offset below 0 (not enough new samples)");
			//add exact amount of 0s to set head back to 0 (+ loopback safety buffer)
			for (int i = _totalNewSamples - _loopbackInfo.nextCalculationSample; i < 0; i++) {
				loopbackDataPtr->add(0);
			}
			_totalNewSamples += _loopbackInfo.nextCalculationSample - _totalNewSamples;

			_historyStartPosition = Vis::consts._loopbackCacheSafetyBuffer; //reevaluate start position
		}

		if (_historyStartPosition >= Vis::consts._requiredLoopbackCacheLength - Vis::consts._loopbackCacheSafetyBuffer) {
			Vengine::warning("sample gathering offset too high (calculation sample not kept up with new samples)");
			resetLoopback(_totalNewSamples);

			if (_numLagsInTimeWindow == 0) {
				Vengine::MyTiming::resetTimer(_loopbacklagTimerId);
				Vengine::MyTiming::startTimer(_loopbacklagTimerId);
				Vengine::warning("Starting lag check");
			}

			_numLagsInTimeWindow++;

			//more resets than allowed in time window then assume lagging and reduce CPS
			if (Vengine::MyTiming::readTimer(_loopbacklagTimerId) < Vis::consts._timeWindowForLag && _numLagsInTimeWindow >= Vis::consts._numLagsBeforeReducingCPS) {
				Vis::comms._wasCPSautoDecreased = true;
				Vis::vars._desiredCPS *= Vis::consts._CPSreduceFactor; //decrease desired cps

				Vengine::warning("Lag detected!");
				UIglobalFeatures::queueError("Cannot reach desired audio calculations per second (CPS), reducing CPS to " + std::to_string(Vis::vars._desiredCPS)); //show error

				_numLagsInTimeWindow = 0;
				Vengine::MyTiming::resetTimer(_loopbacklagTimerId);
			}
		}

		if (_numLagsInTimeWindow > 0 && Vengine::MyTiming::readTimer(_loopbacklagTimerId) >= Vis::consts._timeWindowForLag) {
			//not lagging and been above time window
			Vengine::MyTiming::resetTimer(_loopbacklagTimerId);
			Vengine::warning("Finished lag check- " + std::to_string(_numLagsInTimeWindow) + " resets in " + std::to_string(Vis::consts._timeWindowForLag) + "s");
			_numLagsInTimeWindow = 0;
		}
		//get reverse and put it into sample data array
		for (int i = _historyStartPosition; i < _historyStartPosition + Vis::consts._finalLoopbackStorageSize; i++) {
			_loopbackInfo.sampleDataArrayPtr[_historyStartPosition + Vis::consts._finalLoopbackStorageSize - 1 - i] = _volume * miniaudio.getLoopbackDataHistoryPtr(numNewSamples)->get(i);
		}

		return;
	}

	//if not using loopback ---------

	_loadedAudioInfo.sampleCounter = miniaudio.getCurrentSample();
	if (miniaudio.isFinished()) {
		_loadedAudioInfo.isAudioPlaying = false;
	}

	//update nextcalculationsample and then decide whether to do signal processing--
	if (_loadedAudioInfo.doSignalProcessing) //if did signal processing last frame
		_loadedAudioInfo.nextCalculationSample += miniaudio.getSampleRate() / Vis::vars._desiredCPS; //increment calculation sample

	//will only do calculation if sample counter bigger than calculation sample and will wait for loopback samples to catch up if not
	_loadedAudioInfo.doSignalProcessing = (_loadedAudioInfo.nextCalculationSample <= _loadedAudioInfo.sampleCounter);
	if (!_loadedAudioInfo.doSignalProcessing)
		Vengine::MyTiming::resetTimer(_fileAudioLagTimerId);
	//--

	_loadedAudioInfo.sampleDataArrayStartPosition = _loadedAudioInfo.nextCalculationSample;
	_loadedAudioInfo.isAudioLoadedThisFrame = false;

	//if next calculation sample behind for too long then assume lagging
	if (_loadedAudioInfo.nextCalculationSample < _loadedAudioInfo.sampleCounter) {
		Vengine::MyTiming::startTimer(_fileAudioLagTimerId);
	}
	if (_loadedAudioInfo.nextCalculationSample > _loadedAudioInfo.sampleCounter) {
		Vengine::MyTiming::resetTimer(_fileAudioLagTimerId);
	}

	if (_loadedAudioInfo.sampleCounter - _loadedAudioInfo.nextCalculationSample > Vis::consts._minAmountNextCalculationSampleBehind &&
		Vengine::MyTiming::readTimer(_fileAudioLagTimerId) > Vis::consts._minTimeNextCalculationSampleBehind)
	{
		Vis::vars._desiredCPS *= Vis::consts._CPSreduceFactor; //decrease desired cps
		UIglobalFeatures::queueError("Cannot reach desired audio calculations per second (CPS), reducing CPS to " + std::to_string(Vis::vars._desiredCPS)); //show error

		Vis::comms._wasCPSautoDecreased = true;
		Vengine::MyTiming::resetTimer(_fileAudioLagTimerId);

		SignalProcessingManager::reset();
	}
}

void AudioManager::setUseLoopback(bool useLoopback) {

	_isUsingLoopback = useLoopback;

	if (_isUsingLoopback) {
		pause();

		miniaudio.stopLoopback();
		miniaudio.startLoopback(_loopbackInfo.sampleRate, _loopbackDeviceName);

		_currentPlaybackInfo = &_loopbackInfo;

		Vengine::MyTiming::resetTimer(_loopbackTimerId);
		Vengine::MyTiming::startTimer(_loopbackTimerId);
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

	if (miniaudio.isFinished()) {
		seekToSample(0);
	}

	_loadedAudioInfo.isAudioPlaying = true;
	miniaudio.playAudio();

	audioInterruptOccured(getCurrentSample());
}

void AudioManager::pause()
{
	_loadedAudioInfo.isAudioPlaying = false;
	miniaudio.pauseAudio();

	audioInterruptOccured(getCurrentSample());
}

void AudioManager::seekToSample(int sample) {

	if (_loadedAudioInfo.isAudioLoaded) {
		//sanitise input
		if (sample < 0)
			sample = 0;
		if (sample >= _loadedAudioInfo.sampleDataArrayLength)
			sample = _loadedAudioInfo.sampleDataArrayLength - 1;

		miniaudio.seekToSample(sample);

		audioInterruptOccured(sample);
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
		resetLoopback(currentSample);
	}
	else {
		_loadedAudioInfo.sampleDataArrayStartPosition = currentSample;
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