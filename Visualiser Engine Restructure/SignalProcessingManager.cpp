#include "SignalProcessingManager.h"
#include "VisualiserShaderManager.h"
#include "FourierTransformManager.h"

#include "UIglobalFeatures.h"

#include <functional>
#include <Vengine/DrawFunctions.h>

Master* SignalProcessingManager::_master = nullptr;
NoteOnset* SignalProcessingManager::_noteOnset = nullptr;
TempoDetection* SignalProcessingManager::_tempoDetection = nullptr;
MFCCs* SignalProcessingManager::_mfccs = nullptr;
SimilarityMatrixHandler* SignalProcessingManager::_similarityMatrix = nullptr;

bool SignalProcessingManager::_isFirstReset = true;
int SignalProcessingManager::_lagTimerId = -1;

bool SignalProcessingManager::_hasBeenComputeInterrupt = false;
int SignalProcessingManager::_nextCalculationSample = 0;

void SignalProcessingManager::init() {

	if (!AudioManager::isAudioLoaded()) {
		Vengine::warning("Cannot init with no audio loaded");
		return;
	}

	Vengine::MyTiming::createTimer(_lagTimerId);

	_master = new Master();
	_master->init(AudioManager::getSampleData(), AudioManager::getSampleRate());

	initAlgorithmObjects(true, true, true, true);
}

void SignalProcessingManager::reset()
{
	computeAudioInterrupt();

	if (!AudioManager::isAudioLoaded()) {
		Vengine::warning("Cannot reset as no audio loaded");
		return;
	}

	SP::vars._wasSignalProcessingReset = true;

	_master->reInit(AudioManager::getSampleData(), AudioManager::getSampleRate()); //will tell all other algorithms to reinit
	FourierTransformManager::reInitAll();

	initAlgorithmObjects(true, true, true, true);
}

void SignalProcessingManager::calculate()
{
	if (AudioManager::isAudioLoadedThisFrame()) { //change in song
		Vengine::warning("Change in audio source, reseting signal processing");
		reset(); //reset
	}

	if (!AudioManager::isAudioPlaying()) {
		_master->audioIsPaused();
		return;
	}

	//dependencies
	if (SP::vars._computeTempoDetection) { SP::vars._computeNoteOnset = true; }
	if (SP::vars._computeSimilarityMatrix && _similarityMatrix->isRealTime()) { SP::vars._computeMFCCs = true; }
	if (SP::vars._computeNoteOnset && SP::vars._onsetDetectionFunctionEnum == NoteOnset::DataExtractionAlg::SIM_MATRIX_MEL_SPEC) { SP::vars._computeMFCCs = true; }

	if (_hasBeenComputeInterrupt) {
		std::cout << "Interrupt " << AudioManager::getCurrentSample() << std::endl;
		_nextCalculationSample = AudioManager::getCurrentSample();
		_hasBeenComputeInterrupt = false;
	}
	else if (_nextCalculationSample > AudioManager::getCurrentSample() || _nextCalculationSample >= AudioManager::getNumSamples()) {
		return;
	}

	_master->beginCalculations(int(_nextCalculationSample));
	_master->calculatePeakAmplitude();
	_master->calculateEnergy();

	//all other signal processing done between begin and end--

	if (SP::vars._computeFourierTransform) {
		_master->calculateFourierTransform();
		FourierTransformManager::calculateFourierTransforms();
	}
	if (SP::vars._computeNoteOnset) {
		_noteOnset->calculateNext(NoteOnset::DataExtractionAlg(SP::vars._onsetDetectionFunctionEnum), SP::vars._convolveOnsetDetection);
	}
	if (SP::vars._computeTempoDetection) {
		_tempoDetection->calculateNext();
	}
	if (SP::vars._computeMFCCs) {
		_mfccs->calculateNext();
	}
	if (SP::vars._computeSimilarityMatrix) {
		_similarityMatrix->calculateNext();
	}
	//--

	_master->endCalculations();

	//set vis values that will be sent to shader
	VisualiserShaderManager::updateUniformValuesToOutput();

	//increment sample
	_nextCalculationSample += float(AudioManager::getSampleRate()) / SP::vars._desiredCPS;

	//time how long calculation lags behind frames--
	if (_nextCalculationSample < AudioManager::getCurrentSample()) {
		Vengine::MyTiming::startTimer(_lagTimerId);
	}
	if (_nextCalculationSample > AudioManager::getCurrentSample()) {
		Vengine::MyTiming::resetTimer(_lagTimerId);
	}
	//--


	//if lag for too long reduce cps--
	if (Vengine::MyTiming::readTimer(_lagTimerId) > SP::consts._lagTimeBeforeReducingCPS) {
		SP::vars._desiredCPS *= SP::consts._CPSreduceFactor; //decrease desired cps
		UIglobalFeatures::queueError("Cannot reach desired audio calculations per second (CPS), reducing CPS to " + std::to_string(SP::vars._desiredCPS)); //show error

		reset(); 

		SP::vars._wasCPSautoDecreased = true;
		Vengine::MyTiming::resetTimer(_lagTimerId);
	}
	//--
}


//private------------

void SignalProcessingManager::initAlgorithmObjects(bool noteOnset, bool tempoDetection, bool mfccs, bool similarityMatrix)
{
	if (_master == nullptr) {
		Vengine::fatalError("Cannot create algorithm objects when master = nullptr");
	}

	//MFCCs
	if (mfccs) {
		if (_mfccs != nullptr) {
			_mfccs->reInit();
		}
		else {
			_mfccs = new MFCCs();
			_mfccs->init(_master, SP::consts._numMelBands, 0, 20000);
		}
	}

	//Note onset
	if (noteOnset) {
		if (_noteOnset != nullptr) {
			_noteOnset->reInit();
		}
		else {
			_noteOnset = new NoteOnset(SP::consts._generalHistorySize);
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
			_similarityMatrix->reInit();
		}
		else {
			_similarityMatrix = new SimilarityMatrixHandler(SP::consts._generalHistorySize);
			_similarityMatrix->init(_master);
		}
	}
}
