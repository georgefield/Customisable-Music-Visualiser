#pragma once
#include <stdio.h>
#include <iostream>
#include <Vengine/MyErrors.h>
#include "Limiter.h"


template <class T>
class History
{
public:
	History(int size) : _start(size - 1), _size(size), _addCalls(0), _L(1,0.2,5) {
		_data = new T[_size];
		memset(_data, NULL, _size * sizeof(T));

		_assocSampleData = new int[_size];
		memset(_assocSampleData, -1, _size * sizeof(int)); //-1 for no entry
	}
	~History() {
		delete[] _data;
		delete[] _assocSampleData;
	}
	

	void add(T value, int currentSample = -1) {
		_start = (_start == 0 ? _size - 1 : _start - 1);
		_data[_start] = value;
		_addCalls++;
	};

	void addWithLimiter(T value, int currentSample = -1, int gain = 1){
		add(_L.limitValue(value, gain), currentSample);
	}

	void clear(bool eraseData = false) {
		_start = _size - 1;
		_addCalls = 0;
		if (eraseData) { memset(_data, NULL, _size * sizeof(T)); memset(_data, -1, _size * sizeof(int)); }
	}

	//getters
	T oldest() { return get(std::min(_addCalls - 1, _size - 1)); }
	T newest() { return get(0); }
	T get(int recency) { 
		if (_addCalls == 0) { Vengine::fatalError("No entries in history"); }
		return _data[(_start + recency) % _size]; 
	}
	
	int oldestSample() { return getSample(std::min(_addCalls - 1, _size - 1)); };
	int newestSample() { return getSample(0); }
	int getSample(int recency) {
		if (_addCalls == 0) { Vengine::fatalError("No entries in history"); }
		return _assocSampleData[(_start + recency) % _size];
	}

	//              Start
	//                V                                //Start
	//[1][1][0][1][0][0][0][1][1][0][1]                // V
	T* firstPartPtr() { return &(_data[_start]); } //[0][0][1][1][0][1]
	int firstPartSize() { return _size - _start; }
	T* secondPartPtr() { return _data; } //[1][1][0][1][0]
	int secondPartSize() { return _start; }

	int totalSize() { return _size; }
	int added() { return _addCalls; }
	int entries() { return std::min(_addCalls, _size); }
	bool full() { return _addCalls >= _size; }
	bool empty() { return _addCalls == 0; }
protected:
	T* _data;
	int* _assocSampleData;

	int _start;
	int _size;

	int _addCalls;

	Limiter _L;
};