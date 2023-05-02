#include "SimilarityMatrixHandler.h"
#include "AudioManager.h"
#include "SignalProcessingManager.h"
#include "VisVars.h"
#include "UIglobalFeatures.h"

#include <Vengine/MyTiming.h>

SimilarityMatrixHandler::SimilarityMatrixHandler(bool useSetters)
	:
	matrix(Vis::consts._generalHistorySize),
	_fourierTransform(nullptr),
	_samplesAheadForFutureMatrix(0),
	_counterForDownscale(0),
	_useSetters(useSetters)
{
}

SimilarityMatrixHandler::~SimilarityMatrixHandler()
{
	if (_fourierTransform != nullptr) {
		delete _fourierTransform;
	}
	if (_useSetters) {
		removeUpdaters();
	}
}

void SimilarityMatrixHandler::init(Master* master)
{
	_master = master;

	_samplesAheadForFutureMatrix = _SMinfo._downscale * (float(_SMinfo._matrixSize) / (2.0f * Vis::vars._desiredCPS)) * _master->_sampleRate;

	matrix.init(_SMinfo._matrixSize);

	_futureMaster.init(AudioManager::_currentPlaybackInfo->sampleRate, false);

	_futureMFCCs.init(&_futureMaster, Vis::consts._numMelBands, 0, 20000, false);

	//fourier transform inited when created
	if (_useSetters) {
		initUpdaters();
	}
}

void SimilarityMatrixHandler::reInit()
{
	_samplesAheadForFutureMatrix = _SMinfo._downscale * (float(_SMinfo._matrixSize) / (2.0f * Vis::vars._desiredCPS)) * _master->_sampleRate;

	matrix.reInit(_SMinfo._matrixSize);
	relinkBasedOnSPvars();

	_futureMaster.reInit(AudioManager::_currentPlaybackInfo->sampleRate);

	_futureMFCCs.reInit();

	if (_SMinfo._linkedTo == FT) {
		_fourierTransform->reInit();
	}
}

void SimilarityMatrixHandler::calculateNext()
{
	if (AudioManager::isUsingLoopback() && _SMinfo._useFuture) {
		_SMinfo._useFuture = false;
		UIglobalFeatures::queueError("Cannot use future matrix when using PC audio");
	}

	if (!AudioManager::isUsingLoopback() && AudioManager::_currentPlaybackInfo->sampleCounter + _samplesAheadForFutureMatrix > AudioManager::_currentPlaybackInfo->sampleDataArrayLength) {
		Vengine::warning("Cannot calculate future as samples ahead puts it above song size");
		return;
	}

	if (_SMinfo._linkedTo == NONE) {
		return; //dont bother doing anything if not linked
	}
	
	//calculate only 1 out of every resolution factor
	if (_counterForDownscale != 0) {
		_counterForDownscale = (_counterForDownscale + 1) % _SMinfo._downscale;
		return;
	}
	_counterForDownscale = (_counterForDownscale + 1) % _SMinfo._downscale;

	if (_SMinfo._useFuture) {

		//std::cout << "Samples ahead: " << _samplesAhead << std::endl;
		_futureMaster.beginCalculations(
			AudioManager::_currentPlaybackInfo->nextCalculationSample + _samplesAheadForFutureMatrix,
			&AudioManager::_currentPlaybackInfo->sampleDataArrayPtr[AudioManager::_currentPlaybackInfo->sampleDataArrayStartPosition + _samplesAheadForFutureMatrix],
			AudioManager::_currentPlaybackInfo->sampleDataArrayLength - (AudioManager::_currentPlaybackInfo->sampleDataArrayStartPosition + _samplesAheadForFutureMatrix)
		);

		//handling calculating future of audio features for similarity matrix--
		_futureMaster.calculateFourierTransform(); //ft needed for both

		if (_SMinfo._linkedTo == MFCC || _SMinfo._linkedTo == MelBandEnergies || _SMinfo._linkedTo == MelSpectrogram) {
			_futureMFCCs.calculateNext();
		}
		if (_SMinfo._linkedTo == FT) {
			_fourierTransform->calculateNext();
		}
		//--

		matrix.calculateNext(_SMinfo._measureType, _SMinfo._contrastFactor);

		_futureMaster.endCalculations();
	}
	else {
		//if not in future then already handled apart from FT
		if (_SMinfo._linkedTo == FT) {
			_fourierTransform->calculateNext();
		}

		matrix.calculateNext(_SMinfo._measureType, _SMinfo._contrastFactor);
	}
}

void SimilarityMatrixHandler::relinkBasedOnSPvars()
{
	if (_SMinfo._linkedTo == NONE) {
		return;
	}

	if (_SMinfo._linkedTo == MFCC) {
		linkToMFCCs(_SMinfo._coeffLow, _SMinfo._coeffHigh);
		return;
	}
	if (_SMinfo._linkedTo == MelBandEnergies) {
		linkToMelBandEnergies();
		return;
	}
	if (_SMinfo._linkedTo == MelSpectrogram) {
		linkToMelSpectrogram();
		return;
	}
	if (_SMinfo._linkedTo == FT) {
		linkToFourierTransform(_SMinfo._cutoffLow, _SMinfo._cutoffHigh, _SMinfo._cutoffSmoothFactor);
		return;
	}
}

void SimilarityMatrixHandler::linkToMFCCs(int coeffLow, int coeffHigh)
{
	_SMinfo._coeffLow = coeffLow;
	_SMinfo._coeffHigh = coeffHigh;

	_SMinfo._linkedTo = MFCC;
	if (_SMinfo._useFuture) {
		matrix.linkToMFCCs(&_futureMFCCs, coeffLow, coeffHigh);
		return;
	}
	matrix.linkToMFCCs(SignalProcessingManager::_mfccs, coeffLow, coeffHigh);
}

void SimilarityMatrixHandler::linkToMelBandEnergies()
{
	_SMinfo._linkedTo = MelBandEnergies;
	if (_SMinfo._useFuture) {
		matrix.linkToMelBandEnergies(&_futureMFCCs);
		return;
	}
	matrix.linkToMelBandEnergies(SignalProcessingManager::_mfccs);
}

void SimilarityMatrixHandler::linkToMelSpectrogram()
{
	_SMinfo._linkedTo = MelSpectrogram;
	if (_SMinfo._useFuture) {
		matrix.linkToMelSpectrogram(&_futureMFCCs);
		return;
	}
	matrix.linkToMelSpectrogram(SignalProcessingManager::_mfccs);
}

void SimilarityMatrixHandler::linkToFourierTransform(float cutoffLow, float cutoffHigh, float smoothFactor)
{
	_SMinfo._linkedTo = FT;

	_SMinfo._cutoffLow = cutoffLow;
	_SMinfo._cutoffHigh = cutoffHigh;
	_SMinfo._cutoffSmoothFactor = smoothFactor;

	//delete old fourier transform if exists
	if (_fourierTransform != nullptr) {
		delete _fourierTransform;
	}
	//create new fourier transform
	_fourierTransform = new FourierTransform(1, cutoffLow, cutoffHigh, smoothFactor);

	if (_SMinfo._useFuture) {
		_fourierTransform->init(&_futureMaster, -1);
	}
	else {
		_fourierTransform->init(_master, -1);
	}

	//link it
	matrix.linkToFourierTransform(_fourierTransform);
}


void SimilarityMatrixHandler::initUpdaters()
{
	std::function<float()> similarityMeasureUpdaterFunction = std::bind(&SelfSimilarityMatrix::getSimilarityMeasure, &matrix);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_similarityMeasure", similarityMeasureUpdaterFunction);
}

void SimilarityMatrixHandler::removeUpdaters()
{
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_similarityMeasure");
}
