#pragma once
#include "History.h"

class FourierTransformHistory {

public:
	FourierTransformHistory(int historySize):
		_history(historySize),
		_added(0),
		_numHarmonics(-1)
	{

	}

	void init(int numHarmonics) {
		_numHarmonics = numHarmonics;

		for (int i = 0; i < _history.totalSize(); i++) { //allocate memory for history (init memory to 0)
			_history.add(new float[_numHarmonics]);
			memset(_history.newest(), 0.0f, (_numHarmonics * sizeof(float)));
		}
	}


	~FourierTransformHistory(){
		if (_numHarmonics != -1) { //only delete if initialised
			for (int i = 0; i < _history.totalSize(); i++) { //avoid memory leak
				delete[] _history.get(i);
			}
		}
	}

	//unique functions
	float* workingArray() //modify values here in memory
	{
		if (_numHarmonics == -1) {
			Vengine::fatalError("FourierTransformHistory used without being initialised");
		}
		return _history.oldest(); //work on oldest entries
	}

	void addWorkingArrayToHistory(int currentSample = -1) //call after setting to add to front of history
	{
		_history.add(_history.oldest(), currentSample);
		_added++;
	}

	int numHarmonics() {
		return _numHarmonics;
	}

	//same functionality as history required
	float* get(int index) {
		if (_numHarmonics == -1) {
			Vengine::fatalError("FourierTransformHistory used without being initialised");
		}
		return _history.get(index);
	}
	float* newest() {
		return _history.get(0);
	}
	float* previous() {
		return _history.get(1);
	}

	int totalSize() {
		return _history.totalSize();
	}

	int entries() {
		return std::min(_added, totalSize());
	}
//private:
	int _added;
	int _numHarmonics;
	History<float*> _history;
};