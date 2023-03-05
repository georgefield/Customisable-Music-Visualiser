#pragma once
#include "MFCCs.h"
#include "FourierTransform.h"
#include "Tools.h"
#include "History.h"

#include <Vengine/MyTiming.h>
#include "SimilarityMatrixStructure.h"


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
	}

	void init(int matrixSize) {
		_initialised = true;

		_similarityMatrix = new SimilarityMatrixStructure(matrixSize); //default small correlation (computationally expensive)
		_similarityMatrix->init();
	}

	void reInit(int matrixSize) {
		_linkedTo = NONE;
		if (matrixSize != _similarityMatrix->sideLength()) {
			delete _similarityMatrix;
			_similarityMatrix = new SimilarityMatrixStructure(matrixSize);
			_similarityMatrix->init();
		}
		else {
			_similarityMatrix->reInit();
		}
	}

	void setCorrelationWindowSize(int windowSize) { //reduces noise (smooths matrix), computationally expensive
		_similarityMatrix->setCorrelationWindowSize(windowSize);
	}


	void calculateNext(); 

	void linkToMFCCs(MFCCs* mfcc, int coeffLow, int coeffHigh);
	void linkToMelBandEnergies(MFCCs* mfcc);
	void linkToMelSpectrogram(MFCCs* mfcc);
	void linkToFourierTransform(FourierTransform* ft);
	void linkToDebug();

	//getters

	float getSimilarityMeasure() {
		return _similarityMeasureHistory.newest();
	}
	History<float>* getSimilarityMeasureHistory() {
		return &_similarityMeasureHistory;
	}

	float getSelfSimilarityMatrixValue(int i, int j) {
		return _similarityMatrix->get(i, j);
	}

	void debug();
private:
	void calculateSimilarityMeasure();

	bool _initialised;

	static enum LinkedTo {
		NONE,
		DEBUG,
		MFCC,
		MelBandEnergies,
		MelSpectrogram,
		FT
	};

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
};

