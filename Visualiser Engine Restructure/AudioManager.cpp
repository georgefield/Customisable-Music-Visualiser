#include "AudioManager.h"
#include "VisualiserShaderManager.h"
#include <functional>

Vengine::Audio AudioManager::_audio;
std::string AudioManager::_audioFilepath = "";

void AudioManager::createUniformSetterFunctions()
{
	std::function<int()> currentSampleSetterFunc = AudioManager::getCurrentSample;
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Current Sample", currentSampleSetterFunc);

	std::function<int()> sampleRateSetterFunc = AudioManager::getSampleRate;
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Sample Rate", sampleRateSetterFunc);
}

void AudioManager::loadAudio(std::string filePath)
{
	_audio.loadWav(filePath);
	_audio.queueLoadedWav();
	_audioFilepath = filePath;
}

void AudioManager::play()
{
	if (isAudioFinished()) { 
		_audio.queueLoadedWav(); 
	} 
	
	_audio.play();
}

void AudioManager::restart()
{
	_audio.queueLoadedWav();
	_audio.play();
}
