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
float SignalProcessingManager::_nextCalculationSample = 0;
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

	//set up general history size uniform setter
	std::function<int()> generalHistorySizeSetterFunction = SignalProcessingManager::getGeneralHistorySize;
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("General history size", generalHistorySizeSetterFunction);
}

void SignalProcessingManager::reset()
{
	if (!AudioManager::isAudioLoaded()) {
		Vengine::warning("Cannot reset as no audio loaded");
		return;
	}

	SPvars::UI::_wasSignalProcessingReset = true;

	_nextCalculationSample = 0;

	_master->reInit(AudioManager::getSampleData(), AudioManager::getSampleRate()); //will tell all other algorithms to reinit
	FourierTransformManager::reInitAll();

	initAlgorithmObjects(true, true, true, true, true);
}

void SignalProcessingManager::calculate()
{
	if (AudioManager::isAudioLoadedThisFrame()) { //change in song
		Vengine::warning("Change in audio source, reseting signal processing");
		reset(); //reset
	}

	if (!AudioManager::isAudioPlaying()) {
		return;
	}

	//dependencies
	if (SPvars::UI::_computeTempoDetection) { SPvars::UI::_computeNoteOnset = true; }
	if (SPvars::UI::_computeSimilarityMatrix && _similarityMatrix->isRealTime()) { SPvars::UI::_computeMFCCs = true; }
	if (SPvars::UI::_computeNoteOnset && SPvars::UI::_onsetDetectionFunctionEnum == NoteOnset::DataExtractionAlg::SIM_MATRIX_MEL_SPEC) { SPvars::UI::_computeMFCCs = true; }

	if (_nextCalculationSample > AudioManager::getCurrentSample()) {
		
		return;
	}

	_master->beginCalculations(int(_nextCalculationSample));

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

	//increment sample
	_nextCalculationSample += float(AudioManager::getSampleRate()) / SPvars::UI::_desiredCPS;

	//time how long calculation lags behind frames--
	if (_nextCalculationSample < AudioManager::getCurrentSample()) {
		Vengine::MyTiming::startTimer(_lagTimerId);
	}
	if (_nextCalculationSample > AudioManager::getCurrentSample()) {
		Vengine::MyTiming::resetTimer(_lagTimerId);
	}
	//--


	//if lag for too long reduce cps--
	if (Vengine::MyTiming::readTimer(_lagTimerId) > SPvars::Const::_lagTimeBeforeReducingCPS) {
		SPvars::UI::_desiredCPS *= SPvars::Const::_CPSreduceFactor; //decrease desired cps
		UIglobalFeatures::queueError("Cannot reach desired audio calculations per second (CPS), reducing CPS to " + std::to_string(SPvars::UI::_desiredCPS)); //show error

		reset(); //sets it to _nextCalculationSample to 0

		//so need to catch up
		while (_nextCalculationSample < AudioManager::getCurrentSample()) {
			_nextCalculationSample += float(AudioManager::getSampleRate()) / SPvars::UI::_desiredCPS;
		}

		SPvars::UI::_wasCPSautoDecreased = true;
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
			_rms = new RMS(SPvars::Const::_generalHistorySize);
			_rms->init(_master);
		}
	}

	std::cout << "RMS tick" << std::endl;

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

	std::cout << "MFCCs tick" << std::endl;

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

	std::cout << "Note onset tick" << std::endl;

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

	std::cout << "tempo tick" << std::endl;


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

	std::cout << "Sim mat tick" << std::endl;

}
