#pragma once
#include "MFCCs.h"
#include "FourierTransform.h"
#include "Tools.h"
#include "History.h"

#include <Vengine/MyTiming.h>
#include "SimilarityMatrixStructure.h"
#include "SimMatrixStructs.h"


class SelfSimilarityMatrix
{
public:


	SelfSimilarityMatrix(int historySize) :
		_linkedTo(NONE),
		_mfccsPtr(nullptr),
		_ftPtr(nullptr),

		_similarityMeasureHistory(historySize),
		_initialised(false)
	{
		Vengine::MyTiming::createTimer(_debugTimerId);
	}

	void init(int matrixSize) {
		_initialised = true;

		_similarityMatrix = new SimilarityMatrixStructure(matrixSize); //default small correlation (computationally expensive)
		_similarityMatrix->init();
	}

	void reInit(int matrixSize) {
		_linkedTo = NONE;

		if (matrixSize != _similarityMatrix->matrixSize()) {
			delete _similarityMatrix;
			_similarityMatrix = new SimilarityMatrixStructure(matrixSize);
			_similarityMatrix->init();
		}
		else {
			_similarityMatrix->reInit();
		}
	}

	void setCorrelationWindow(int size) {
		if (size < 2) {
			return;
		}
		_similarityMatrix->setCorrelationWindowSize(size);
	}

	void setCorrelationWindowSize(int windowSize) { //reduces noise (smooths matrix), computationally expensive
		_similarityMatrix->setCorrelationWindowSize(windowSize);
	}


	void calculateNext(MeasureType measureType, float contrastFactor);

	void linkToMFCCs(MFCCs* mfcc, int coeffLow, int coeffHigh);
	void linkToMelBandEnergies(MFCCs* mfcc);
	void linkToMelSpectrogram(MFCCs* mfcc);
	void linkToFourierTransform(FourierTransform* ft);
	void linkToDebug();

	//getters

	float getSimilarityMeasure() {
		if (_linkedTo == NONE)
			return 0.0f;
		return _similarityMeasureHistory.newest();
	}
	History<float>* getSimilarityMeasureHistory() {
		return &_similarityMeasureHistory;
	}

	float getSelfSimilarityMatrixValue(int i, int j) {
		return _similarityMatrix->get(i, j);
	}
	int getMatrixSize() {
		return _similarityMatrix->matrixSize();
	}

	Vengine::GLtexture getMatrixTexture() {
		if (!_similarityMatrix->isTextureCreated()) {
			_similarityMatrix->createTexture(SP::vars._fastSimilarityMatrixTexture);
		}
		return _similarityMatrix->getMatrixTexture();
	}
	int getMatrixTextureStartPixelIndex() {
		return _similarityMatrix->textureStartIndex();
	}


	bool isCalculating() { return _initialised && (_linkedTo != NONE); }

	void debug();

	LinkedTo islinkedTo() { return _linkedTo; }

private:

	void calculateSimilarityMeasure();
	void calculatePrecussionMeasure();

	bool _initialised;

	History<float> _similarityMeasureHistory;

	SimilarityMatrixStructure* _similarityMatrix;
	LinkedTo _linkedTo;

	//link ptrs
	MFCCs* _mfccsPtr;
	int _coeffLow, _coeffHigh;
	FourierTransform* _ftPtr;

	//debug
	int _debugTimerId;
	bool _switch;

	float checkerboardKernel(int i, int j);
	float inverseCrossKernel(int i, int j);

};

