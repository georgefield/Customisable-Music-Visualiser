#include "SignalProcessingManager.h"
#include "VisualiserShaderManager.h"
#include "FourierTransformManager.h"

#include <functional>
#include <Vengine/DrawFunctions.h>

Master* SignalProcessingManager::_master = nullptr;
RMS* SignalProcessingManager::_rms = nullptr;
NoteOnset* SignalProcessingManager::_noteOnset = nullptr;
TempoDetection* SignalProcessingManager::_tempoDetection = nullptr;
MFCCs* SignalProcessingManager::_mfccs = nullptr;
SelfSimilarityMatrix* SignalProcessingManager::_selfSimilarityMatrix = nullptr;

bool SignalProcessingManager::UI_computeFourierTransform = true;
bool SignalProcessingManager::UI_computeRms = true;
bool SignalProcessingManager::UI_computeNoteOnset = true;
bool SignalProcessingManager::UI_computeTempoDetection = true;
bool SignalProcessingManager::UI_computeMFCCs = false;
bool SignalProcessingManager::UI_computeSelfSimilarityMatrix = false;

std::string SignalProcessingManager::_currentAudioFilepath = "";

bool SignalProcessingManager::_started = false;

int SignalProcessingManager::GENERAL_HISTORY_SIZE = 1000;

void SignalProcessingManager::start()
{
	if (_started) {
		Vengine::warning("start already called for signal processing manager");
		return;
	}

	if (!ready()) {
		Vengine::warning("Cannot start yet as no audio to play");
		return;
	}

	_master = new Master();
	_master->init(AudioManager::getAudioData(), AudioManager::getSampleRate());

	initAlgorithmObjects(true, true, true, true, true);

	//set up general history size uniform setter
	std::function<int()> generalHistorySizeSetterFunction = SignalProcessingManager::getGeneralHistorySize;
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("General history size", generalHistorySizeSetterFunction);

	//set filepath
	_currentAudioFilepath = AudioManager::getAudioFilepath();

	_started = true;
}

void SignalProcessingManager::restart()
{
	if (!_started) {
		Vengine::warning("SignalProcessing: Cannot call reset before calling start");
		return;
	}

	_master->reInit(AudioManager::getAudioData(), AudioManager::getSampleRate()); //will tell all other algorithms to reinit
	FourierTransformManager::reInitAll();

	initAlgorithmObjects(true, true, true, true, true);

	//set filepath
	_currentAudioFilepath = AudioManager::getAudioFilepath();
}

bool SignalProcessingManager::ready()
{
	return AudioManager::isAudioLoaded();
}

void SignalProcessingManager::calculate()
{
	if (!_started) {
		Vengine::warning("Cannot calculate as signalProcessing not started");
		return;
	}

	if (_currentAudioFilepath != AudioManager::getAudioFilepath()) { //change in song
		Vengine::warning("Change in audio source, reseting signal processing");
		start(); //reset
	}

	//dependencies
	if (UI_computeTempoDetection) { UI_computeNoteOnset = true; }


	if (!AudioManager::isAudioPlaying()) { //no calculations unless audio playing
		return;
	}

	_master->beginCalculations(AudioManager::getCurrentSample());

	//all other signal processing done between begin and end--

	if (UI_computeFourierTransform) {
		_master->calculateFourierTransform();
		FourierTransformManager::calculateFourierTransforms();
	}
	if (UI_computeRms) {
		_rms->calculateNext(4096, LINEAR_PYRAMID);
	}
	if (UI_computeNoteOnset) {
		_noteOnset->calculateNext();
	}
	if (UI_computeTempoDetection) {
		_tempoDetection->calculateNext();
	}
	if (UI_computeMFCCs) {
		_mfccs->calculateNext();
	}
	if (UI_computeSelfSimilarityMatrix) {
		_selfSimilarityMatrix->calculateNext();
	}
	//--

	_master->endCalculations();
}


//private

void SignalProcessingManager::initAlgorithmObjects(bool rms, bool noteOnset, bool tempoDetection, bool mfccs, bool selfSimilarityMatrix)
{
	if (_master == nullptr) {
		Vengine::fatalError("Cannot create algorithm objects when master = nullptr");
	}
	/// <summary>
	/// Change in 
	/// </summary>
	if (rms) {
		if (_rms != nullptr) {
			_rms->reInit();
		}
		else {
			_rms = new RMS(GENERAL_HISTORY_SIZE);
			_rms->init(_master);
		}
	}


	if (noteOnset) {
		if (_noteOnset != nullptr) {
			_noteOnset->reInit();
		}
		else {
			_noteOnset = new NoteOnset(GENERAL_HISTORY_SIZE);
			_noteOnset->init(_master);
		}
	}



	if (tempoDetection) {
		if (_tempoDetection != nullptr) {
			_tempoDetection->reInit();
		}
		else {
			_tempoDetection = new TempoDetection();
			_tempoDetection->init(_master, _noteOnset);
		}
	}


	if (mfccs) {
		if (_mfccs != nullptr) {
			_mfccs->reInit();
		}
		else {
			_mfccs = new MFCCs();
			_mfccs->init(_master, 25, 0, 20000);
		}
	}

	if (selfSimilarityMatrix) {
		if (_selfSimilarityMatrix != nullptr) {
		}
		else {
			_selfSimilarityMatrix = new SelfSimilarityMatrix(1000, GENERAL_HISTORY_SIZE);
		}
	}

}
