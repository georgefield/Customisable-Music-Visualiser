#pragma once
#include <stdio.h>
#include <iostream>
#include <Vengine/MyErrors.h>

#include <vector>

template <class T>
class History
{
public:
	History(int size) : _start(size - 1), _size(size), _entries(0), _usingContiguousArray(false) {
		_data = new T[_size];
		memset(_data, NULL, _size * sizeof(T));
	}
	~History() {
		delete[] _data;
		if (_usingContiguousArray) {
			delete[] _contiguousArray;
		}
	}
	

	void add(T value) {
		_start = (_start == 0 ? _size - 1 : _start - 1);
		_data[_start] = value;
		_entries = (_entries >= _size ? _entries : _entries + 1);
	};

	void removeOldest() {
		_entries = (_entries > 0 ? _entries - 1 : 0);
	}

	void clear(bool eraseData = false) {
		_start = _size - 1;
		_entries = 0;
		if (eraseData) { memset(_data, NULL, _size * sizeof(T)); memset(_data, -1, _size * sizeof(int)); }
	}

	void resize(int size) {

		T* tmpData = new T[size];
		getAsContiguousArray();
		memcpy(tmpData, _contiguousArray, std::min(size, _size) * sizeof(T)); //copy ordered array to new data array

		_usingContiguousArray = false;
		delete[] _contiguousArray; //delete contiguous array

		if (size > _size) //if new size bigger than current fill rest with NULL
			memset(&tmpData[_size], NULL, (size - _size) * sizeof(T));

		delete[] _data; //free old data memory
		_data = tmpData; //point to resized array

		_start = 0; //start is now 0 (as got as contiguous array)
		_size = size; //set new size

	}

	//getters
	std::vector<T> getAsVector(bool newestFirst = true, int firstXentries = -1) {
		std::vector<T> toReturn;
		if (firstXentries == -1) { //get all
			firstXentries = entries();
		}
		else if (firstXentries > entries()) {
			Vengine::warning("Requested more entries than history has as a vector, defaulting to all entries");
			firstXentries = entries();
		}
		toReturn.reserve(firstXentries);

		if (newestFirst) {
			for (int i = 0; i < firstXentries; i++) {
				toReturn.push_back(get(i));
			}
		}
		else {
			for (int i = firstXentries - 1; i >=0; i--) {
				toReturn.push_back(get(i));
			}
		}

		return toReturn;
	}

	float* getAsContiguousArray() {
		if (!_usingContiguousArray) { //only allocate memory for this on first time
			_contiguousArray = new float[totalSize()];
			_usingContiguousArray = true;
		}

		memcpy(_contiguousArray, firstPartPtr(), firstPartSize() * sizeof(float));
		memcpy(&_contiguousArray[firstPartSize()], secondPartPtr(), secondPartSize() * sizeof(float));

		return _contiguousArray;
	}

	T oldest() { return get(_entries - 1); }
	T newest() { return get(0); }
	T previous() { return get(1); }
	T get(int recency) { 
		if (_entries == 0) { 
			Vengine::warning("Get called on history that has no entries"); 
		}
		return _data[(_start + recency) % _size]; 
	}

	//              Start
	//                V                                //Start
	//[1][1][0][1][0][0][0][1][1][0][1]                // V
	T* firstPartPtr() { return &(_data[_start]); } //[0][0][1][1][0][1]
	int firstPartSize() { return _size - _start; }
	T* secondPartPtr() { return _data; } //[1][1][0][1][0]
	int secondPartSize() { return _start; }
	T* dataStartPtr() { return _data; } //same as second part ptr but easier to understand
	int firstPartOffset() { return _start; } //also same as second part size

	int totalSize() { return _size; }
	int added() { return _entries; }
	int entries() { return _entries; }
	bool full() { return _entries >= _size; }
	bool empty() { return _entries == 0; }

	void debug() {
		for (int i = 0; i < totalSize(); i++) {
			std::cout << get(i) << std::endl;
		}
	}
protected:
	T* _data;

	bool _usingContiguousArray;
	float* _contiguousArray;

	int _start;
	int _size;

	int _entries;
};