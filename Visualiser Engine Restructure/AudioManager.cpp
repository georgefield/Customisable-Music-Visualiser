#include "AudioManager.h"
#include "VisualiserShaderManager.h"
#include <functional>
#include <Vengine/IOManager.h>
#include <assert.h>
#include "SignalProcessingManager.h"

#include "imgui.h"

miniaudio_api AudioManager::miniaudio;
std::string AudioManager::_audioFileName = "";

bool AudioManager::_audioLoadedThisFrame = false;

int AudioManager::_currentSampleExtraPrecisionTimerId;
int AudioManager::_stickySample = -1;
int AudioManager::_currentSample = 0;

float AudioManager::_volume = 1.0f;

void AudioManager::init()
{
	miniaudio.init();
	Vengine::MyTiming::createTimer(_currentSampleExtraPrecisionTimerId);
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
	_audioLoadedThisFrame = true;

	_currentSample = 0;
	_stickySample = -1;

	SignalProcessingManager::audioInterruptOccured(0);

	return true;
}

void AudioManager::play()
{
	if (miniaudio.isLoaded()) {
		miniaudio.playAudio();

		SignalProcessingManager::audioInterruptOccured(_currentSample);
	}
}

void AudioManager::pause()
{
	if (miniaudio.isLoaded()) {
		miniaudio.pauseAudio();

		SignalProcessingManager::audioInterruptOccured(_currentSample);
	}
}

void AudioManager::seekToSample(int sample) {

	if (miniaudio.isLoaded()){
		//sanitise input
		if (sample < 0)
			sample = 0;
		if (sample >= getNumSamples())
			sample = getNumSamples() - 1;

		miniaudio.seekToSample(sample);

		//reset current sample
		_currentSample = sample;
		_stickySample = -1;

		SignalProcessingManager::audioInterruptOccured(sample);
	}
}


float* AudioManager::getSampleData()
{
	if (miniaudio.isLoaded()) {
		return miniaudio.getAudioData();
	}
	return nullptr;
}

int AudioManager::getNumSamples()
{
	if (miniaudio.isLoaded()) {
		return miniaudio.getAudioDataLength();
	}
	return -1;
}

int AudioManager::getCurrentSample()
{
	if (miniaudio.isLoaded()) {

		if (isAudioPlaying()) {
			if (_stickySample != miniaudio.getCurrentSample()) { //if mini audio not stuck on same sample
				_stickySample = miniaudio.getCurrentSample(); //update sticky sample

				Vengine::MyTiming::resetTimer(_currentSampleExtraPrecisionTimerId); //start timer to get extra 
				Vengine::MyTiming::startTimer(_currentSampleExtraPrecisionTimerId);
			}

			//add extra samples for extra precision
			int extraSamples = Vengine::MyTiming::readTimer(_currentSampleExtraPrecisionTimerId) * getSampleRate();
			_currentSample = miniaudio.getCurrentSample() + extraSamples;
			
			//check not over num samples
			if (_currentSample > getNumSamples()) {
				_currentSample = getNumSamples() - 1;
			}
		}

		return _currentSample;
	}
	return -1;
}

int AudioManager::getSampleRate()
{
	if (miniaudio.isLoaded()) {
		return miniaudio.getSampleRate();
	}

	return 0;
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
		overrideSample = getCurrentSample();
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



void AudioManager::setVolume(float volume) {
	if (!miniaudio.isLoaded()) {
		return;
	}

	assert(volume >= 0.0f && volume <= 1.0f);

	if (volume != _volume) {
		_volume = volume;
		miniaudio.setVolume(_volume);
	}
}


bool AudioManager::isAudioPlaying()
{
	if (miniaudio.isLoaded()) {
		return miniaudio.isPlaying() && !miniaudio.isFinished();
	}
	return false;
}

bool AudioManager::isAudioFinished() {

	if (miniaudio.isLoaded()) {
		return miniaudio.isFinished();
	}
	return true;
}

bool AudioManager::isAudioLoadedThisFrame()
{
	if (!miniaudio.isLoaded()) {
		_audioLoadedThisFrame = false;
	}

	if (_audioLoadedThisFrame) {
		_audioLoadedThisFrame = false;
		return true;
	}
	return false;
}

bool AudioManager::isAudioLoaded()
{
	return miniaudio.isLoaded();
}

void AudioManager::showImguiDebugWindow()
{
	ImGui::Begin("Audio debug");

	if (miniaudio.isLoaded()) {
		std::string info = "Sample rate: " + std::to_string(getSampleRate()) + ", Current sample: " + std::to_string(getCurrentSample()) + " / " + std::to_string(getNumSamples());
		ImGui::Text(info.c_str());
		ImGui::PlotLines("data", &(getSampleData()[std::max(0, AudioManager::getCurrentSample() - AudioManager::getSampleRate())]), getSampleRate(), 0, 0, -1, 1, ImGui::GetContentRegionAvail());
	}
	else {
		ImGui::Text("No audio loaded");
	}
	ImGui::End();
}