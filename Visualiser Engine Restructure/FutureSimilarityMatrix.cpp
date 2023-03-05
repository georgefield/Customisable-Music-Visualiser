#include "FutureSimilarityMatrix.h"
#include "AudioManager.h"
#include "SignalProcessingManager.h"
#include "SignalProcessingVars.h"

FutureSimilarityMatrix::FutureSimilarityMatrix(int size)
	:
matrix(SPvars::Const::_generalHistorySize),
_fourierTransform(nullptr)
{
}

FutureSimilarityMatrix::~FutureSimilarityMatrix()
{
	if (_fourierTransform != nullptr) {
		delete _fourierTransform;
	}
}

void FutureSimilarityMatrix::init(int matrixSize)
{
	_matrixSize = matrixSize;
	_samplesAhead = ((matrixSize / 60) / 2) * AudioManager::getSampleRate(); //samples per frame * (matrixSize/2), means the middle of the matrix will be current sample

	_futureMaster.init(AudioManager::getAudioData(), AudioManager::getSampleRate(), false);

	_futureMFCCs.init(&_futureMaster, SPvars::Const::_numMelBands, 0, 20000, false);

	//fourier transform inited when created

	matrix.init(matrixSize);
}

void FutureSimilarityMatrix::reInit(int matrixSize)
{
	_matrixSize = matrixSize;
	_samplesAhead = ((matrixSize / 60) / 2) * AudioManager::getSampleRate(); //samples per frame * (matrixSize/2), means the middle of the matrix will be current sample

	_futureMaster.reInit(AudioManager::getAudioData(), AudioManager::getSampleRate());

	_futureMFCCs.reInit();

	if (_linkedTo == FT) {
		_fourierTransform->reInit();
	}

	matrix.reInit(matrixSize);
}

void FutureSimilarityMatrix::calculateNext()
{
	if (AudioManager::getCurrentSample() + _samplesAhead > AudioManager::getAudioDataSize()) {
		Vengine::warning("Cannot calculate future as samples ahead puts it above song size");
		return;
	}

	if (_linkedTo == NONE) {
		return; //dont bother doing anything if not linked
	}

	_futureMaster.beginCalculations(AudioManager::getCurrentSample() + _samplesAhead);

	//calculate for similarity matrix--
	_futureMaster.calculateFourierTransform(); //ft needed for both

	if (_linkedTo == MFCC) {
		_futureMFCCs.calculateNext();
	}
	if (_linkedTo == FT) {
		_fourierTransform->beginCalculation();
		_fourierTransform->endCalculation();
	}
	//--

	matrix.calculateNext();

	_futureMaster.endCalculations();
}

void FutureSimilarityMatrix::linkToMFCCs(int coeffLow, int coeffHigh)
{
	matrix.linkToMFCCs(&_futureMFCCs, coeffLow, coeffHigh);
	_linkedTo = MFCC;
}

void FutureSimilarityMatrix::linkToFourierTransform(float cutoffLow, float cutoffHigh, float smoothFrac)
{
	//delete old fourier transform if exists
	if (_fourierTransform != nullptr) {
		delete _fourierTransform;
	}
	//create new fourier transform
	_fourierTransform = new FourierTransform(1, cutoffLow, cutoffHigh, smoothFrac);
	_fourierTransform->init(&_futureMaster, "");
	//link it
	matrix.linkToFourierTransform(_fourierTransform);
	_linkedTo = FT;
}
