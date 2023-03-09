#include "SimilarityMatrixHandler.h"
#include "AudioManager.h"
#include "SignalProcessingManager.h"
#include "SignalProcessingVars.h"

#include <Vengine/MyTiming.h>

SimilarityMatrixHandler::SimilarityMatrixHandler(int size)
	:
matrix(SPvars::Const::_generalHistorySize),
_fourierTransform(nullptr),
_samplesAheadForFutureMatrix(0)
{
}

SimilarityMatrixHandler::~SimilarityMatrixHandler()
{
	if (_fourierTransform != nullptr) {
		delete _fourierTransform;
	}
	deleteSetters();
}

void SimilarityMatrixHandler::init(Master* master, int matrixSize)
{
	_master = master;
	_matrixSize = matrixSize;

	_samplesAheadForFutureMatrix = (float(matrixSize) / (2.0f * SPvars::UI::_desiredCPS)) * _master->_sampleRate;

	_futureMaster.init(AudioManager::getSampleData(), AudioManager::getSampleRate(), false);

	_futureMFCCs.init(&_futureMaster, SPvars::Const::_numMelBands, 0, 20000, false);

	//fourier transform inited when created

	matrix.init(matrixSize);

	initSetters();
}

void SimilarityMatrixHandler::reInit(int matrixSize)
{
	_matrixSize = matrixSize;
	_samplesAheadForFutureMatrix = (float(matrixSize) / (2.0f * SPvars::UI::_desiredCPS)) * _master->_sampleRate;

	_futureMaster.reInit(AudioManager::getSampleData(), AudioManager::getSampleRate());

	_futureMFCCs.reInit();

	if (_linkedTo == FT) {
		_fourierTransform->reInit();
	}

	matrix.reInit(matrixSize);
}

void SimilarityMatrixHandler::calculateNext()
{
	if (AudioManager::getCurrentSample() + _samplesAheadForFutureMatrix > AudioManager::getNumSamples()) {
		Vengine::warning("Cannot calculate future as samples ahead puts it above song size");
		return;
	}

	if (_linkedTo == NONE) {
		return; //dont bother doing anything if not linked
	}
	
	if (SPvars::UI::_useFutureSimilarityMatrix) {

		//std::cout << "Samples ahead: " << _samplesAhead << std::endl;
		_futureMaster.beginCalculations(AudioManager::getCurrentSample() + _samplesAheadForFutureMatrix);

		//handling calculating future of audio features for similarity matrix--
		_futureMaster.calculateFourierTransform(); //ft needed for both

		if (_linkedTo == MFCC || _linkedTo == MelBandEnergies || _linkedTo == MelSpectrogram) {
			_futureMFCCs.calculateNext();
		}
		if (_linkedTo == FT) {
			_fourierTransform->beginCalculation();
			_fourierTransform->endCalculation();
		}
		//--

		matrix.calculateNext(MeasureType(SPvars::UI::_matrixMeasureEnum), SPvars::UI::_similarityMatrixTextureContrastFactor);

		_futureMaster.endCalculations();
	}
	else {
		//if not in future then already handled apart from FT
		if (_linkedTo == FT) {
			_fourierTransform->beginCalculation();
			_fourierTransform->endCalculation();
		}

		matrix.calculateNext(MeasureType(SPvars::UI::_matrixMeasureEnum), SPvars::UI::_similarityMatrixTextureContrastFactor);
	}

}

void SimilarityMatrixHandler::linkToMFCCs(int coeffLow, int coeffHigh)
{
	_coeffLow = coeffLow;
	_coeffHigh = coeffHigh;

	_linkedTo = MFCC;
	if (SPvars::UI::_useFutureSimilarityMatrix) {
		matrix.linkToMFCCs(&_futureMFCCs, coeffLow, coeffHigh);
		return;
	}
	matrix.linkToMFCCs(SignalProcessingManager::_mfccs, coeffLow, coeffHigh);
}

void SimilarityMatrixHandler::linkToMelBandEnergies()
{
	_linkedTo = MelBandEnergies;
	if (SPvars::UI::_useFutureSimilarityMatrix) {
		matrix.linkToMelBandEnergies(&_futureMFCCs);
		return;
	}
	matrix.linkToMelBandEnergies(SignalProcessingManager::_mfccs);
}

void SimilarityMatrixHandler::linkToMelSpectrogram()
{
	_linkedTo = MelSpectrogram;
	if (SPvars::UI::_useFutureSimilarityMatrix) {
		matrix.linkToMelSpectrogram(&_futureMFCCs);
		return;
	}
	matrix.linkToMelSpectrogram(SignalProcessingManager::_mfccs);
}

void SimilarityMatrixHandler::linkToFourierTransform(float cutoffLow, float cutoffHigh, float smoothFactor)
{
	_cutoffLow = cutoffLow;
	_cutoffHigh = cutoffHigh;
	_cutoffSmoothFactor = smoothFactor;

	//delete old fourier transform if exists
	if (_fourierTransform != nullptr) {
		delete _fourierTransform;
	}
	//create new fourier transform
	_fourierTransform = new FourierTransform(1, cutoffLow, cutoffHigh, smoothFactor);

	if (SPvars::UI::_useFutureSimilarityMatrix) {
		_fourierTransform->init(&_futureMaster, "");
	}
	else {
		_fourierTransform->init(_master, "");
	}

	//link it
	matrix.linkToFourierTransform(_fourierTransform);
	_linkedTo = FT;
}


void SimilarityMatrixHandler::initSetters()
{
	//energy setters init in energy class
	std::function<int()> matrixSizeSetterFunction = std::bind(&SelfSimilarityMatrix::getMatrixSize, &matrix);
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Similarity matrix size", matrixSizeSetterFunction);

	std::function<int()> matrixStartIndexForTextureSetterFunction = std::bind(&SelfSimilarityMatrix::getMatrixTextureStartPixelIndex, &matrix);
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Similarity matrix texture start point", matrixStartIndexForTextureSetterFunction);
}

void SimilarityMatrixHandler::deleteSetters()
{
	VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("Similarity matrix size");
	VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("Similarity matrix texture start point");
}
