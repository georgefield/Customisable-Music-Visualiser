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
SimilarityMatrixHandler* SignalProcessingManager::_similarityMatrix = nullptr;

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
	if (SPvars::UI::_computeTempoDetection) { SPvars::UI::_computeNoteOnset = true; }
	if (SPvars::UI::_computeSimilarityMatrix && _similarityMatrix->isRealTime()) { SPvars::UI::_computeMFCCs = true; }

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
		_noteOnset->calculateNext(NoteOnset::DataExtractionAlg(SPvars::UI::_onsetDetectionFunctionEnum), SPvars::UI::_convolveOnsetDetection);
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
	//--

	_master->endCalculations();
}


//private

void SignalProcessingManager::initAlgorithmObjects(bool rms, bool noteOnset, bool tempoDetection, bool mfccs, bool similarityMatrix)
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

	//Note onset
	if (noteOnset) {
		if (_noteOnset != nullptr) {
			_noteOnset->reInit();
		}
		else {
			_noteOnset = new NoteOnset(SPvars::Const::_generalHistorySize);
			_noteOnset->init(_master, _mfccs);
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

	//Similarity matrix
	if (similarityMatrix) {
		if (_similarityMatrix != nullptr) {
			_similarityMatrix->reInit(SPvars::UI::_nextSimilarityMatrixSize);
		}
		else {
			_similarityMatrix = new SimilarityMatrixHandler(SPvars::Const::_generalHistorySize);
			_similarityMatrix->init(_master, SPvars::UI::_nextSimilarityMatrixSize);
		}
	}
}
