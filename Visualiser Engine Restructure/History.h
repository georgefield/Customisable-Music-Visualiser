#pragma once
#include <stdio.h>
#include <iostream>
#include <Vengine/MyErrors.h>

template <class T>
class History
{
public:
	History(int size) : _start(size - 1), _size(size), _addCalls(0) {
		_data = (T*)malloc(_size * sizeof(T));
		memset(_data, NULL, _size * sizeof(T));
	}
	~History() {
		free(_data);
	}
	
	void add(T value) {
		_start = (_start == 0 ? _size - 1 : _start - 1);
		_data[_start] = value;
		_addCalls++;
	};

	void clear(bool eraseData = false) {
		_start = _size - 1;
		_addCalls = 0;
		if (eraseData) { memset(_data, NULL, _size * sizeof(T)); }
	}

	//getters
	T oldest() { return get(std::min(_addCalls - 1, _size - 1)); }
	T newest() { return get(0); }
	T get(int recency) { 
		if (_addCalls == 0) { Vengine::fatalError("No entries in history"); }
		return _data[(_start + recency) % _size]; 
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
private:
	T* _data;
	int _start;
	int _size;

	int _addCalls;
};