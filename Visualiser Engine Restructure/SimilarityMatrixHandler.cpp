#include "SimilarityMatrixHandler.h"
#include "AudioManager.h"
#include "SignalProcessingManager.h"
#include "SignalProcessingVars.h"

#include <Vengine/MyTiming.h>

SimilarityMatrixHandler::SimilarityMatrixHandler(int size)
	:
matrix(SPvars::Const::_generalHistorySize),
_fourierTransform(nullptr),
_samplesAhead(0)
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

	_samplesAhead = 0; //samples per frame * (matrixSize/2), means the middle of the matrix will be current sample

	_futureMaster.init(AudioManager::getAudioData(), AudioManager::getSampleRate(), false);

	_futureMFCCs.init(&_futureMaster, SPvars::Const::_numMelBands, 0, 20000, false);

	//fourier transform inited when created

	matrix.init(matrixSize);

	initSetters();
}

void SimilarityMatrixHandler::reInit(int matrixSize)
{
	_matrixSize = matrixSize;
	_samplesAhead = 0; //samples per frame * (matrixSize/2), means the middle of the matrix will be current sample

	_futureMaster.reInit(AudioManager::getAudioData(), AudioManager::getSampleRate());

	_futureMFCCs.reInit();

	if (_linkedTo == FT) {
		_fourierTransform->reInit();
	}

	matrix.reInit(matrixSize);
}

void SimilarityMatrixHandler::calculateNext()
{
	if (AudioManager::getCurrentSample() + _samplesAhead > AudioManager::getAudioDataSize()) {
		Vengine::warning("Cannot calculate future as samples ahead puts it above song size");
		return;
	}

	if (_linkedTo == NONE) {
		return; //dont bother doing anything if not linked
	}
	
	if (_samplesAhead > 0) {

		//std::cout << "Samples ahead: " << _samplesAhead << std::endl;
		_futureMaster.beginCalculations(AudioManager::getCurrentSample() + _samplesAhead);

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

		matrix.calculateNext(MeasureType(SPvars::UI::_matrixMeasureEnum));

		_futureMaster.endCalculations();
	}
	else {
		//if not in future then already handled apart from FT
		if (_linkedTo == FT) {
			_fourierTransform->beginCalculation();
			_fourierTransform->endCalculation();
		}

		matrix.calculateNext(MeasureType(SPvars::UI::_matrixMeasureEnum));
	}

}

void SimilarityMatrixHandler::linkToMFCCs(int coeffLow, int coeffHigh)
{
	_coeffLow = coeffLow;
	_coeffHigh = coeffHigh;

	_linkedTo = MFCC;
	if (_samplesAhead > 0) {
		matrix.linkToMFCCs(&_futureMFCCs, coeffLow, coeffHigh);
		return;
	}
	matrix.linkToMFCCs(SignalProcessingManager::_mfccs, coeffLow, coeffHigh);
}

void SimilarityMatrixHandler::linkToMelBandEnergies()
{
	_linkedTo = MelBandEnergies;
	if (_samplesAhead > 0) {
		matrix.linkToMelBandEnergies(&_futureMFCCs);
		return;
	}
	matrix.linkToMelBandEnergies(SignalProcessingManager::_mfccs);
}

void SimilarityMatrixHandler::linkToMelSpectrogram()
{
	_linkedTo = MelSpectrogram;
	if (_samplesAhead > 0) {
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

	if (_samplesAhead > 0) {
		_fourierTransform->init(&_futureMaster, "");
	}
	else {
		_fourierTransform->init(_master, "");
	}

	//link it
	matrix.linkToFourierTransform(_fourierTransform);
	_linkedTo = FT;
}

void SimilarityMatrixHandler::setLookAhead(int samples)
{
	_samplesAhead = samples;
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
