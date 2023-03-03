#pragma once
#include "VectorHistory.h"

//interface for vector history but with some function names changed to make code more readable when using
class FourierTransformHistory {

public:
	FourierTransformHistory(int historySize) :
		_history(historySize)
	{}

	void init(int numHarmonics) {

		_history.init(numHarmonics);
	}

	void reInit() {
		_history.reInit();
	}

	float* workingArray()
	{
		return _history.workingArray();
	}

	void addWorkingArrayToHistory(int currentSample = -1)
	{
		_history.addWorkingArrayToHistory(currentSample);
	}

	int numHarmonics() {
		return _history.vectorDim();
	}

	//same functionality as history required
	float* get(int index) { return _history.get(index); }

	float* newest() { return _history.get(0); }

	float* previous() { return _history.get(1); }

	int totalSize() { return _history.totalSize(); }

	int entries() { return _history.entries(); }
private:
	VectorHistory _history;
};