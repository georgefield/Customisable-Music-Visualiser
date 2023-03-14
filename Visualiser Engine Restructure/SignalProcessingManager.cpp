#include "SignalProcessingManager.h"
#include "VisualiserShaderManager.h"
#include "FourierTransformManager.h"

#include "UIglobalFeatures.h"

#include <functional>
#include <Vengine/DrawFunctions.h>

Master* SignalProcessingManager::_master = nullptr;
RMS* SignalProcessingManager::_rms = nullptr;
NoteOnset* SignalProcessingManager::_noteOnset = nullptr;
TempoDetection* SignalProcessingManager::_tempoDetection = nullptr;
MFCCs* SignalProcessingManager::_mfccs = nullptr;
SimilarityMatrixHandler* SignalProcessingManager::_similarityMatrix = nullptr;

bool SignalProcessingManager::_isFirstReset = true;
int SignalProcessingManager::_lagTimerId = -1;

void SignalProcessingManager::init() {

	if (!AudioManager::isAudioLoaded()) {
		Vengine::warning("Cannot init with no audio loaded");
		return;
	}

	Vengine::MyTiming::createTimer(_lagTimerId);

	_master = new Master();
	_master->init(AudioManager::getSampleData(), AudioManager::getSampleRate());

	initAlgorithmObjects(true, true, true, true, true);
}

void SignalProcessingManager::reset()
{
	if (!AudioManager::isAudioLoaded()) {
		Vengine::warning("Cannot reset as no audio loaded");
		return;
	}

	SPvars._wasSignalProcessingReset = true;

	_master->reInit(AudioManager::getSampleData(), AudioManager::getSampleRate()); //will tell all other algorithms to reinit
	FourierTransformManager::reInitAll();

	initAlgorithmObjects(true, true, true, true, true);
}

void SignalProcessingManager::calculate()
{
	if (AudioManager::isAudioLoadedThisFrame()) { //change in song
		Vengine::warning("Change in audio source, reseting signal processing");
		SPvars._nextCalculationSample = 0;
		reset(); //reset
	}

	if (!AudioManager::isAudioPlaying()) {
		_master->audioIsPaused();
		return;
	}

	//dependencies
	if (SPvars._computeTempoDetection) { SPvars._computeNoteOnset = true; }
	if (SPvars._computeSimilarityMatrix && _similarityMatrix->isRealTime()) { SPvars._computeMFCCs = true; }
	if (SPvars._computeNoteOnset && SPvars._onsetDetectionFunctionEnum == NoteOnset::DataExtractionAlg::SIM_MATRIX_MEL_SPEC) { SPvars._computeMFCCs = true; }

	if (SPvars._nextCalculationSample > AudioManager::getCurrentSample()) {
		
		return;
	}

	_master->beginCalculations(int(SPvars._nextCalculationSample));
	_master->calculatePeakAmplitude();
	_master->calculateEnergy();

	//all other signal processing done between begin and end--

	if (SPvars._computeFourierTransform) {
		_master->calculateFourierTransform();
		FourierTransformManager::calculateFourierTransforms();
	}
	if (SPvars._computeRms) {
		_rms->calculateNext(4096, LINEAR_PYRAMID);
	}
	if (SPvars._computeNoteOnset) {
		_noteOnset->calculateNext(NoteOnset::DataExtractionAlg(SPvars._onsetDetectionFunctionEnum), SPvars._convolveOnsetDetection);
	}
	if (SPvars._computeTempoDetection) {
		_tempoDetection->calculateNext();
	}
	if (SPvars._computeMFCCs) {
		_mfccs->calculateNext();
	}
	if (SPvars._computeSimilarityMatrix) {
		_similarityMatrix->calculateNext();
	}
	//--

	_master->endCalculations();

	//set vis values that will be sent to shader
	VisualiserShaderManager::updateUniformValuesToOutput();

	//increment sample
	SPvars._nextCalculationSample += float(AudioManager::getSampleRate()) / SPvars._desiredCPS;

	//time how long calculation lags behind frames--
	if (SPvars._nextCalculationSample < AudioManager::getCurrentSample()) {
		Vengine::MyTiming::startTimer(_lagTimerId);
	}
	if (SPvars._nextCalculationSample > AudioManager::getCurrentSample()) {
		Vengine::MyTiming::resetTimer(_lagTimerId);
	}
	//--


	//if lag for too long reduce cps--
	if (Vengine::MyTiming::readTimer(_lagTimerId) > SPvars._lagTimeBeforeReducingCPS) {
		SPvars._desiredCPS *= SPvars._CPSreduceFactor; //decrease desired cps
		UIglobalFeatures::queueError("Cannot reach desired audio calculations per second (CPS), reducing CPS to " + std::to_string(SPvars._desiredCPS)); //show error

		reset(); 

		while (SPvars._nextCalculationSample < AudioManager::getCurrentSample()) {
			SPvars._nextCalculationSample += float(AudioManager::getSampleRate()) / SPvars._desiredCPS;
		}

		SPvars._wasCPSautoDecreased = true;
		Vengine::MyTiming::resetTimer(_lagTimerId);
	}
	//--
}


//private------------

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
			_rms = new RMS(SPvars._generalHistorySize);
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
			_mfccs->init(_master, SPvars._numMelBands, 0, 20000);
		}
	}

	//Note onset
	if (noteOnset) {
		if (_noteOnset != nullptr) {
			_noteOnset->reInit();
		}
		else {
			_noteOnset = new NoteOnset(SPvars._generalHistorySize);
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
			_similarityMatrix->reInit(SPvars._nextSimilarityMatrixSize);
		}
		else {
			_similarityMatrix = new SimilarityMatrixHandler(SPvars._generalHistorySize);
			_similarityMatrix->init(_master, SPvars._nextSimilarityMatrixSize);
		}
	}
}
