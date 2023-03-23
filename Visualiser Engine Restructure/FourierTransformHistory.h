#pragma once
#include "VectorHistory.h"

//interface for vector history but with some function names changed to make code more readable when using
class FourierTransformHistory {

public:
	FourierTransformHistory(int historySize) :
		_vecHistory(historySize)
	{}

	void init(int numHarmonics) {

		_vecHistory.init(numHarmonics);
	}

	void changeNumHarmonics(int newNumHarmonics) {
		_vecHistory.reInit(newNumHarmonics);
	}

	float* workingArray()
	{
		return _vecHistory.workingArray();
	}

	void addWorkingArrayToHistory()
	{
		_vecHistory.addWorkingArrayToHistory();
	}

	int numHarmonics() {
		return _vecHistory.vectorDim();
	}

	//same functionality as history required
	float* get(int index) { return _vecHistory.get(index); }

	float* newest() { return _vecHistory.get(0); }

	float* previous() { return _vecHistory.get(1); }

	int totalSize() { return _vecHistory.totalSize(); }

	int entries() { return _vecHistory.entries(); }
private:
	VectorHistory<float> _vecHistory;
};