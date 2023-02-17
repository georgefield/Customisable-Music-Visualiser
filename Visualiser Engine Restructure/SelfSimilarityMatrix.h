#pragma once
#include "MFCCs.h"
#include "FourierTransform.h"
#include "Tools.h"
#include "History.h"

#include <glm/glm.hpp>

struct IntCoord {
	int x, y;
};

class SimilarityMatrixStructure {
public:
	SimilarityMatrixStructure(int sideLength, int correlationWindowSize = 1) :
		_sideLength(sideLength),
		_correlationWindowSize(correlationWindowSize),
		_start(_sideLength - 1), //start br and move up to tl then wrap around

		_data(nullptr),
		_dataWindowedCorrelation(nullptr) //init to nullptr as might not be initialised in constructor
	{
		_useCorrelationWindow = (_correlationWindowSize <= 1); // size 1 correlation window data = data, only needed for size 2 and up

		//main data
		_data = new float* [_sideLength];
		for (int i = 0; i < _sideLength; i++) {
			_data[i] = new float[_sideLength];
			memset(_data[i], 0.0f, _sideLength * sizeof(float));
		}

		if (!_useCorrelationWindow) {
			return;
		}

		//only set aside memory if using correlation window
		_dataWindowedCorrelation = new float* [_sideLength];
		for (int i = 0; i < _sideLength; i++) {
			_dataWindowedCorrelation[i] = new float[_sideLength];
			memset(_data[i], 0.0f, _sideLength * sizeof(float));
		}
	}

	~SimilarityMatrixStructure() {
		for (int i = 0; i < _sideLength; i++) {
			delete[] _data[i];
		}
	}

	void add(std::vector<float> v1) {
		_start--;
		_entries = std::min(_entries + 1, _sideLength);

		for (int i = 0; i < _sideLength; i++) {
			(*getPtrToCoord(_start + i, _start, false)) = similarityMeasure(v1, v2);
		}
		for (int i = 0; i < _sideLength; i++) {
			(*getPtrToCoord(_start, _start + i, false)) = similarityMeasure(v1, v2);
		}

		if (!_useCorrelationWindow) {
			return;
		}

		//correlation window must be done after basic similarity matrix updated as is an average
		for (int i = 0; i < _sideLength; i++) {
			(*getPtrToCoord(_start + i, _start, true)) = correlationOverWindow(_start + i, _start);
		}
		for (int i = 0; i < _sideLength; i++) {
			(*getPtrToCoord(_start, _start + i, true)) = correlationOverWindow(_start, _start + i);
		}
	}

	void clear() {
		_start = 0;
		_entries = 0;
	}

	//getters

	float get(int i, int j) { //only to be used from outside of class, use get ptr inside
		if (i > _entries || j > _entries) {
			return 0.0f; //
		}

		if (_correlationWindowSize == -1) {
			return *getPtrToCoord(i, j, false);
		}
		return *getPtrToCoord(i, j, true);
	}

	int sideLength() const { return _sideLength; }
	bool full() const { return _entries == _sideLength; }
	int entries() const { return _entries; }
private:
	int _sideLength;
	int _start;
	int _entries;

	float** _data; //store s(i,j)

	bool _useCorrelationWindow;
	int _correlationWindowSize;
	float** _dataWindowedCorrelation; //store 1/w * sum{k = [0,w - 1]]}(s(i + k, j + k))
	


	float* getPtrToCoord(int i, int j, bool getWindowedCorrelation) {
		if (getWindowedCorrelation) {
			return &_dataWindowedCorrelation[(_start + i) % _sideLength][(_start + j) % _sideLength];
		}

		return &_data[(_start + i) % _sideLength][(_start + j) % _sideLength];

	}

	float similarityMeasure(std::vector<float> v1, std::vector<float> v2) {
		if (v1.size() != v2.size()) {
			Vengine::fatalError("Vector sizes sent for similarity measure have unequal dimension");
		}

		float dot = Tools::dot(&(v1[0]), &(v2[0]), v1.size());
		float n1 = Tools::L2norm(&(v1[0]), &(v1[0]), v1.size());
		float n2 = Tools::L2norm(&(v2[0]), &(v2[0]), v2.size());

		return dot / (n1 * n2);
	}


	float correlationOverWindow(int x, int y) {
		float sum = 0;
		for (int i = 0; i < _correlationWindowSize; i++) {
			if (x + i < _entries && y + i < _entries) { //only include valid values
				sum += *getPtrToCoord(x + i, y + i, false);
			}
		}
		sum /= float(_correlationWindowSize);
		return sum;
	}
};


class SimilarityMatrix
{
public:

	SimilarityMatrix(Master* m, int size) :
		_m(m),
		_similarityMatrix(size),

		_linkedTo(NONE),
		_mfccsPtr(nullptr),
		_ftPtr(nullptr)
	{
	}

	void calculateNext();

	void linkToMFCCs(MFCCs* mfcc);
	void linkToMelBandEnergies(MFCCs* mfcc);
	void linkToMelSpectrogram(MFCCs* mfcc);
	void linkToFourierTransform(FourierTransform* ft);

private:
	static enum LinkedTo {
		NONE,
		MFCC,
		MelBandEnergies,
		MelSpectrogram,
		FT
	};

	Master* _m;
	SimilarityMatrixStructure _similarityMatrix;
	LinkedTo _linkedTo;

	//link ptrs
	MFCCs* _mfccsPtr;
	FourierTransform* _ftPtr;
};

