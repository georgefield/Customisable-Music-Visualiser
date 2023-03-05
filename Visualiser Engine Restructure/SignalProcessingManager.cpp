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
SelfSimilarityMatrix* SignalProcessingManager::_similarityMatrix = nullptr;
FutureSimilarityMatrix* SignalProcessingManager::_futureSimilarityMatrix = nullptr;

std::string SignalProcessingManager::_currentAudioFilepath = "";

bool SignalProcessingManager::_started = false;

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

	initAlgorithmObjects(true, true, true, true, true, true);

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

	initAlgorithmObjects(true, true, true, true, true, true);

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
	if (SPvars::UI::_computeTempoDetection) { SPvars::UI::_computeNoteOnset = true; }


	if (!AudioManager::isAudioPlaying()) { //no calculations unless audio playing
		return;
	}

	_master->beginCalculations(AudioManager::getCurrentSample());

	//all other signal processing done between begin and end--

	if (SPvars::UI::_computeFourierTransform) {
		_master->calculateFourierTransform();
		FourierTransformManager::calculateFourierTransforms();
	}
	if (SPvars::UI::_computeRms) {
		_rms->calculateNext(4096, LINEAR_PYRAMID);
	}
	if (SPvars::UI::_computeNoteOnset) {
		_noteOnset->calculateNext();
	}
	if (SPvars::UI::_computeTempoDetection) {
		_tempoDetection->calculateNext();
	}
	if (SPvars::UI::_computeMFCCs) {
		_mfccs->calculateNext();
	}
	if (SPvars::UI::_computeSimilarityMatrix) {
		_similarityMatrix->calculateNext();
	}
	if (SPvars::UI::_computeFutureSimilarityMatrix) {
		_futureSimilarityMatrix->calculateNext();
	}
	//--

	_master->endCalculations();
}


//private

void SignalProcessingManager::initAlgorithmObjects(bool rms, bool noteOnset, bool tempoDetection, bool mfccs, bool similarityMatrix, bool futureSimilarityMatrix)
{
	if (_master == nullptr) {
		Vengine::fatalError("Cannot create algorithm objects when master = nullptr");
	}

	//RMS
	if (rms) {
		if (_rms != nullptr) {
			_rms->reInit();
		}
		else {
			_rms = new RMS(SPvars::Const::_generalHistorySize);
			_rms->init(_master);
		}
	}

	//Note onset
	if (noteOnset) {
		if (_noteOnset != nullptr) {
			_noteOnset->reInit();
		}
		else {
			_noteOnset = new NoteOnset(SPvars::Const::_generalHistorySize);
			_noteOnset->init(_master);
		}
	}

	//Tempo detection
	if (tempoDetection) {
		if (_tempoDetection != nullptr) {
			_tempoDetection->reInit();
		}
		else {
			_tempoDetection = new TempoDetection();
			_tempoDetection->init(_master, _noteOnset);
		}
	}

	//MFCCs
	if (mfccs) {
		if (_mfccs != nullptr) {
			_mfccs->reInit();
		}
		else {
			_mfccs = new MFCCs();
			_mfccs->init(_master, SPvars::Const::_numMelBands, 0, 20000);
		}
	}

	//Similarity matrix
	if (similarityMatrix) {
		if (_similarityMatrix != nullptr) {
			_similarityMatrix->reInit(SPvars::UI::_nextSimilarityMatrixSize);
		}
		else {
			_similarityMatrix = new SelfSimilarityMatrix(SPvars::Const::_generalHistorySize);
			_similarityMatrix->init(SPvars::UI::_nextSimilarityMatrixSize);
		}
	}

	//Future similarity matrix
	if (futureSimilarityMatrix) {
		if (_futureSimilarityMatrix != nullptr) {
			_futureSimilarityMatrix->reInit(SPvars::UI::_nextSimilarityMatrixSize);
		}
		else {
			_futureSimilarityMatrix = new FutureSimilarityMatrix(SPvars::Const::_generalHistorySize);
			_futureSimilarityMatrix->init(SPvars::UI::_nextSimilarityMatrixSize);
		}
	}
}
