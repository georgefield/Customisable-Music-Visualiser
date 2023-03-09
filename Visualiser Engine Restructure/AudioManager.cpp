#include "AudioManager.h"
#include "VisualiserShaderManager.h"
#include <functional>
#include <Vengine/IOManager.h>

#include "imgui.h"

miniaudio_api AudioManager::miniaudio;
std::string AudioManager::_audioFileName = "";

bool AudioManager::_audioLoadedThisFrame = false;

int AudioManager::_currentSampleExtraPrecisionTimerId;
int AudioManager::_stickySample = 0;
int AudioManager::_currentSample = 0;

void AudioManager::init()
{
	miniaudio.init();
	Vengine::MyTiming::createTimer(_currentSampleExtraPrecisionTimerId);
	createUniformSetterFunctions();
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

	return true;
}

void AudioManager::play()
{
	if (miniaudio.isLoaded()) {
		miniaudio.playAudio();
	}
}

void AudioManager::pause()
{
	if (miniaudio.isLoaded()) {
		miniaudio.pauseAudio();
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

		//if finished return length
		if (miniaudio.isFinished()) {
			_currentSample = miniaudio.getAudioDataLength() - 1;
			return _currentSample;
		}

		//if paused return the current sample or the  miniaudio current sample if its bigger
		if (!miniaudio.isPlaying()){
			_currentSample = std::max(_currentSample, miniaudio.getCurrentSample());
			return _currentSample;
		}

		//if playing, interpolate between miniaudio sticky samples by restarting a timer every time miniaudio correctly updates
		if (_stickySample != miniaudio.getCurrentSample()) {
			_stickySample = miniaudio.getCurrentSample();

			Vengine::MyTiming::resetTimer(_currentSampleExtraPrecisionTimerId);
			Vengine::MyTiming::startTimer(_currentSampleExtraPrecisionTimerId);

			_currentSample = miniaudio.getCurrentSample();
			return _currentSample;
		}
	
		//if on sticky sample for 2 frame add a time calculated amount of samples
		_currentSample = miniaudio.getCurrentSample() + (Vengine::MyTiming::readTimer(_currentSampleExtraPrecisionTimerId) * miniaudio.getSampleRate());
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

//private-------

void AudioManager::createUniformSetterFunctions()
{
	std::function<int()> currentSampleSetterFunc = AudioManager::getCurrentSample;
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Current Sample", currentSampleSetterFunc);

	std::function<int()> sampleRateSetterFunc = AudioManager::getSampleRate;
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Sample Rate", sampleRateSetterFunc);
}