#pragma once
#include "History.h"
#include <vector>

template <class T>
class VectorHistory {

public:
	VectorHistory(int historySize) :
		_history(historySize),
		_added(0),
		_vectorDim(-1),

		_initialised(false)
	{
	}

	~VectorHistory() {
		if (_vectorDim != -1) { //only delete if initialised
			for (int i = 0; i < _history.totalSize(); i++) { //avoid memory leak
				delete[] _history.get(i);
			}
		}
	}


	//*** initing and resetting vector history ***

	void init(int vectorDim) {
		_vectorDim = vectorDim;

		for (int i = 0; i < _history.totalSize(); i++) { //allocate memory for history (init memory to 0)
			_history.add(new T[_vectorDim]);
			memset(_history.newest(), NULL, (_vectorDim * sizeof(T)));
		}

		_initialised = true;
	}

	void reInit(int vectorDim) { //reinit with different vector size (need to wipe heap malloc)
		if (vectorDim == _vectorDim) {
			clear(); //just call clear as no need to create new array ptrs
			return;
		}

		for (int i = 0; i < _history.totalSize(); i++) {
			delete[] get(i);
		}
		_added = 0;

		init(vectorDim);
	}

	void reInit() { //reinit of same size, just clear
		clear();
	}
	
	void clear() {
		for (int i = 0; i < _history.totalSize(); i++) {
			memset(_history.get(i), NULL, (_vectorDim * sizeof(T)));
		}
		_added = 0;
	}

	//***

	
	//*** working with vector history ***

	T* workingArray() //modify values here in memory
	{
		if (_vectorDim == -1) {
			Vengine::fatalError("Vector history used without being initialised");
		}
		return _history.oldest(); //work on oldest entries
	}

	void addWorkingArrayToHistory(int currentSample = -1) //call after setting working array values to what you want
	{
		_history.add(_history.oldest(), currentSample);
		_added++;
	}

	void add(std::vector<T> v) 
	{
		if (v.size() > _vectorDim) {
			Vengine::fatalError("can only add vectors of same size or less to vector history");
		}

		memcpy(workingArray(), &(v[0]), sizeof(T) * v.size()); //copy to working array
		addWorkingArrayToHistory(); //add to history
	}

	int vectorDim() {
		return _vectorDim;
	}

	//***

	//have functionality of history
	bool isInitialised() const { return _initialised; }

	T* get(int index) {
		if (_vectorDim == -1) {
			Vengine::fatalError("Vector history used without being initialised");
		}
		return _history.get(index);
	}
	T* newest() { return _history.get(0); }
	T* previous() { return _history.get(1); }

	int totalSize() { return _history.totalSize();}
	int entries() { return std::min(_added, totalSize()); }
private:
	int _added;
	int _vectorDim;
	History<T*> _history;

	bool _initialised;
};
