#include "SelfSimilarityMatrix.h"
#include <iomanip>

void SelfSimilarityMatrix::calculateNext(MeasureType measureType, float contrastFactor)
{

	if (_initialised == false) {
		Vengine::fatalError("Cannot calculate similarity matrix without initialising");
	}

	if (_linkedTo == NONE) {
		Vengine::warning("Self similarity matrix not linked to any vector data, no calculation performed");
		return;
	}

	if (_linkedTo == MFCC) {
		if (_coeffLow < 1) {
			Vengine::fatalError("MFCC coefficients start from 1");
		}
		_similarityMatrix->add(&(_mfccsPtr->getMfccs()[_coeffLow - 1]), contrastFactor);
	}
	if (_linkedTo == MelBandEnergies) {
		_similarityMatrix->add(_mfccsPtr->getBandEnergies(), contrastFactor);
	}
	if (_linkedTo == MelSpectrogram) {
		_similarityMatrix->add(_mfccsPtr->getMelSpectrogram(), contrastFactor);
	}
	if (_linkedTo == FT) {
		_similarityMatrix->add(_ftPtr->getOutput(), contrastFactor);
	} 


	if (measureType == SIMILARITY){
		calculateSimilarityMeasure();
	}
	if (measureType == PERCUSSION) {
		calculatePrecussionMeasure();
	}

	if (_linkedTo == DEBUG) {
		std::vector<float> debugVec;
		if (_switch) {

			debugVec.push_back(1);
			debugVec.push_back(0);
		}
		else {
			debugVec.push_back(0);
			debugVec.push_back(1);
		}

		std::cout << Vengine::MyTiming::readTimer(_debugTimerId) << std::endl;
		if (Vengine::MyTiming::readTimer(_debugTimerId) > 1.0) {
			Vengine::MyTiming::resetTimer(_debugTimerId);
			Vengine::MyTiming::startTimer(_debugTimerId);
			_switch = !_switch;
		}

		_similarityMatrix->add(&(debugVec[0]), contrastFactor);
		return;
	}
}

float SelfSimilarityMatrix::checkerboardKernel(int i, int j) {
	float middle = (_similarityMatrix->entries() - 1) / 2.0f;
	int sign = 1;
	if ((i < middle && j > middle) || (i > middle && j < middle)) {
		sign = -1;
	}

	//sqrt(2) * middle is max distance from middle - distance from middle * sign 
	return ((1.414 * middle) - sqrt((i - middle) * (i - middle) + (j - middle) * (j - middle))) * sign / (1.414 * middle); 
}

void SelfSimilarityMatrix::calculateSimilarityMeasure()
{
	float sum = 0;
	for (int i = 0; i < _similarityMatrix->entries(); i++) {
		for (int j = 0; j < i; j++) {
			sum += getSelfSimilarityMatrixValue(i, j) * checkerboardKernel(i, j) * 2;
		}
		sum += getSelfSimilarityMatrixValue(i, i) * checkerboardKernel(i, i);
	}
	sum /= _similarityMatrix->matrixSize() * _similarityMatrix->matrixSize();
	sum = std::max(sum, 0.0f); //clamp >0
	_similarityMeasureHistory.add(sum);
}

const float FACTOR = 0.3333f;

const float ON = FACTOR * FACTOR * 4;
const float OFF = ON - 1;
float SelfSimilarityMatrix::inverseCrossKernel(int i, int j) {
	float u = float(i) / float(getMatrixSize());
	float v = float(j) / float(getMatrixSize());

	if (u < FACTOR || u > 1 - FACTOR){
		if (v < FACTOR) {
			return ON;
		}
		else if (v < 1 - FACTOR) {
			return OFF;
		}
		else {
			return ON;
		}
	}
	else {
		if (v < FACTOR) {
			return OFF;
		}
		else if (v < 1 - FACTOR) {
			return ON;
		}
		else {
			return OFF;
		}
	}

	return 0.0f;
}

void SelfSimilarityMatrix::calculatePrecussionMeasure()
{
	float sum = 0;
	for (int i = 0; i < _similarityMatrix->entries(); i++) {
		for (int j = 0; j < i; j++) {
			sum += getSelfSimilarityMatrixValue(i, j) * inverseCrossKernel(i, j);
		}
		sum += getSelfSimilarityMatrixValue(i, i) * inverseCrossKernel(i, i);
	}

	sum /= _similarityMatrix->matrixSize() * _similarityMatrix->matrixSize();
	sum = std::max(sum, 0.0f); //clamp >0
	_similarityMeasureHistory.add(sum);
}

void SelfSimilarityMatrix::linkToMFCCs(MFCCs* mfcc, int coeffLow, int coeffHigh)
{
	_coeffLow = coeffLow;
	_coeffHigh = coeffHigh;

	//sanity check
	assert(_coeffLow > 0);
	assert(_coeffHigh <= SP::consts._numMelBands);
	assert(_coeffLow < _coeffHigh);

	_mfccsPtr = mfcc;
	_linkedTo = MFCC;
	_similarityMatrix->start(_coeffHigh - _coeffLow + 1);
}

void SelfSimilarityMatrix::linkToMelBandEnergies(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MelBandEnergies;
	_similarityMatrix->start(_mfccsPtr->getNumMelBands());
}

void SelfSimilarityMatrix::linkToMelSpectrogram(MFCCs* mfcc)
{
	_mfccsPtr = mfcc;
	_linkedTo = MelSpectrogram;
	_similarityMatrix->start(_mfccsPtr->getNumMelBands());
}

void SelfSimilarityMatrix::linkToFourierTransform(FourierTransform* ft)
{
	_ftPtr = ft;
	_linkedTo = FT;
	_similarityMatrix->start(_ftPtr->getNumHarmonics());
}

void SelfSimilarityMatrix::linkToDebug()
{
	_linkedTo = DEBUG;
	Vengine::MyTiming::createTimer(_debugTimerId);
	_switch = true;
	_similarityMatrix->start(2);
}

void SelfSimilarityMatrix::debug()
{
	int n = std::min(10, _similarityMatrix->matrixSize());
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			std::cout << std::fixed << std::setprecision(2) << getSelfSimilarityMatrixValue(i, j) << ", ";
		}
		std::cout << std::endl;
	}
	std::cout << "-------" << std::endl;
}
