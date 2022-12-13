#pragma once
#include <stdio.h>
#include <iostream>

template <class T>
class History
{
public:
	History(int size) : _start(size - 1) {
		_size = size;
		_data = (T*)malloc(_size * sizeof(T));
		memset(_data, NULL, _size * sizeof(T));
	}
	
	void add(T value) {
		_start = (_start == 0 ? _size - 1 : _start - 1);
		_data[_start] = value;
	};

	//getters
	T oldest() { return get(totalSize() - 1); }
	T newest() { return get(0); }
	T get(int recency) { return _data[(_start + recency) % _size]; }

	//              Start
	//                V                                //Start
	//[1][1][0][1][0][0][0][1][1][0][1]                // V
	T* firstPartPtr() { return &(_data[_start]); } //[0][0][1][1][0][1]
	int firstPartSize() { return _size - _start; }
	T* secondPartPtr() { return _data; } //[1][1][0][1][0]
	int secondPartSize() { return _start; }
	int totalSize() { return _size; }
private:
	T* _data;
	int _start;
	int _size;
};