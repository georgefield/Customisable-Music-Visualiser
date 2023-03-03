#pragma once
#include "History.h"
#include <vector>

class VectorHistory {

public:
	VectorHistory(int historySize) :
		_history(historySize),
		_added(0),
		_vectorDim(-1)
	{

	}

	void init(int vectorDim) {
		_vectorDim = vectorDim;

		for (int i = 0; i < _history.totalSize(); i++) { //allocate memory for history (init memory to 0)
			_history.add(new float[_vectorDim]);
			memset(_history.newest(), 0.0f, (_vectorDim * sizeof(float)));
		}
	}

	void reInit() {
		//memset all to 0.0f
		for (int i = 0; i < _history.totalSize(); i++) {
			memset(_history.get(i), 0.0f, (_vectorDim * sizeof(float)));
		}
		_added = 0; //reset num added to 0
	}


	~VectorHistory() {
		if (_vectorDim != -1) { //only delete if initialised
			for (int i = 0; i < _history.totalSize(); i++) { //avoid memory leak
				delete[] _history.get(i);
			}
		}
	}

	//unique functions
	float* workingArray() //modify values here in memory
	{
		if (_vectorDim == -1) {
			Vengine::fatalError("FourierTransformHistory used without being initialised");
		}
		return _history.oldest(); //work on oldest entries
	}

	void addWorkingArrayToHistory(int currentSample = -1) //call after setting to add to front of history
	{
		_history.add(_history.oldest(), currentSample);
		_added++;
	}

	void add(std::vector<float> v) 
	{
		if (v.size() > _vectorDim) {
			Vengine::fatalError("can only add vectors of same size or less to vector history");
		}

		memcpy(workingArray(), &(v[0]), sizeof(float) * v.size()); //copy to working array
		addWorkingArrayToHistory(); //add to history
	}

	int vectorDim() {
		return _vectorDim;
	}

	//same functionality as history required
	float* get(int index) {
		if (_vectorDim == -1) {
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
private:
	int _added;
	int _vectorDim;
	History<float*> _history;
};
