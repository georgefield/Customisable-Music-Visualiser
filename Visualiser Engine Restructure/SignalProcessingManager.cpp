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

void SignalProcessingManager::init() {

	if (!AudioManager::_currentPlaybackInfo->isAudioLoaded) {
		Vengine::warning("Cannot init with no audio loaded");
		return;
	}

	_master = new Master();
	_master->init(AudioManager::_currentPlaybackInfo->sampleRate);

	initAlgorithmObjects(true, true, true, true);
}

void SignalProcessingManager::reset()
{
	if (!AudioManager::_currentPlaybackInfo->isAudioLoaded) {
		Vengine::warning("Cannot reset as no audio loaded");
		return;
	}

	AudioManager::audioInterruptOccured(AudioManager::_currentPlaybackInfo->sampleCounter);

	Vis::comms._wasSignalProcessingReset = true;

	//reinit everything--
	_master->reInit(AudioManager::_currentPlaybackInfo->sampleRate);
	FourierTransformManager::reInitAll();

	initAlgorithmObjects(true, true, true, true);
	//--
}

void SignalProcessingManager::calculate()
{
	if (AudioManager::_currentPlaybackInfo->isAudioLoadedThisFrame) { //change in song
		Vengine::warning("Change in audio source, reseting signal processing");
		reset(); //reset
	}

	if (!AudioManager::_currentPlaybackInfo->isAudioPlaying) {
		_master->audioIsPaused(); //called to do any computations required when audio is paused
		return;
	}

	//dependencies
	if (Vis::vars._computeTempoDetection) { Vis::vars._computeNoteOnset = true; }
	if (Vis::vars._computeSimilarityMatrix) { Vis::vars._computeMFCCs = true; }


	//if audio has not caught up with audio hop size on this frame, skip frame
	Vis::comms._isCalculationFrame = AudioManager::_currentPlaybackInfo->doSignalProcessing;
	if (!Vis::comms._isCalculationFrame) {
		return;
	}

	_master->beginCalculations(AudioManager::_currentPlaybackInfo->nextCalculationSample,
		&AudioManager::_currentPlaybackInfo->sampleDataArrayPtr[AudioManager::_currentPlaybackInfo->sampleDataArrayStartPosition],
		AudioManager::_currentPlaybackInfo->sampleDataArrayLength - AudioManager::_currentPlaybackInfo->sampleDataArrayStartPosition
	);
	_master->calculateFourierTransform();
	_master->calculatePeakAmplitude();
	_master->calculateEnergy();

	//all other signal processing done between begin and end--
	if (Vis::vars._computeFrequencyBands) {
		FourierTransformManager::calculateFourierTransforms();
	}
	if (Vis::vars._computeNoteOnset) {
		_noteOnset->calculateNext(NoteOnset::DataExtractionAlg(Vis::vars._onsetDetectionFunctionEnum), Vis::vars._convolveOnsetDetection);
	}
	if (Vis::vars._computeTempoDetection) {
		_tempoDetection->calculateNext();
	}
	if (Vis::vars._computeMFCCs) {
		_mfccs->calculateNext();
	}
	if (Vis::vars._computeSimilarityMatrix) {
		_similarityMatrix->calculateNext();
	}
	//--

	_master->endCalculations();
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
			_mfccs->init(_master, Vis::consts._numMelBands, 0, 20000);
		}
	}

	//Note onset
	if (noteOnset) {
		if (_noteOnset != nullptr) {
			_noteOnset->reInit();
		}
		else {
			_noteOnset = new NoteOnset(Vis::consts._generalHistorySize);
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
			_similarityMatrix = new SimilarityMatrixHandler(true);
			_similarityMatrix->init(_master, _mfccs);
		}
	}
}
