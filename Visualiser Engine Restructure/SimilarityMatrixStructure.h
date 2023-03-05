#pragma once
#include "VectorHistory.h"
#include <Vengine/MyErrors.h>
#include "Tools.h"

class SimilarityMatrixStructure {
public:
	SimilarityMatrixStructure(int matrixSize) :
		_vectorHistory(matrixSize),
		_matrixSize(matrixSize),

		_data(nullptr),
		_dataWindowedCorrelation(nullptr), //init to nullptr as might not be initialised in constructor

		_initialised(false)
	{
	}


	~SimilarityMatrixStructure() {
		if (_initialised) {
			for (int i = 0; i < _matrixSize; i++) {
				delete[] _data[i];
			}
		}
	}

	void init(int correlationWindowSize = 1) {
		_initialised = true;

		_start = 0;

		//set up matrix--
		_data = new float* [_matrixSize];
		for (int i = 0; i < _matrixSize; i++) {
			_data[i] = new float[_matrixSize];
			memset(_data[i], 0.0f, _matrixSize * sizeof(float));
		}
		//--

		//always set aside memory for correlation window matrix as just makes life easier--
		setCorrelationWindowSize(correlationWindowSize);

		_dataWindowedCorrelation = new float* [_matrixSize];
		for (int i = 0; i < _matrixSize; i++) {
			_dataWindowedCorrelation[i] = new float[_matrixSize];
			memset(_dataWindowedCorrelation[i], 0.0f, _matrixSize * sizeof(float));
		}
		//--
	}

	void reInit(int correlationWindowSize = 1) {
		_start = 0;

		_vectorHistory.clear();

		//set up correlation window vars again with the passed parameter--
		setCorrelationWindowSize(correlationWindowSize);
		//--
	}

	void setCorrelationWindowSize(int correlationWindowSize) {
		_correlationWindowSize = correlationWindowSize;
		_useCorrelationWindow = (_correlationWindowSize >= 2); // size 1 correlation window data = data, only needed for size 2 and up

		if (_correlationWindowSize > _matrixSize) {
			Vengine::warning("Correlation window cannot be greater than side length, defaulting to side length");
			_correlationWindowSize = _matrixSize;
		}
	}

	//call start not init when linking to data
	void start(int vectorDim) {

		//set up vector history
		if (_vectorHistory.isInitialised()) {
			_vectorHistory.reInit(vectorDim);
		}
		else {
			_vectorHistory.init(vectorDim);
		}

		_vectorDim = vectorDim;
	}

	void add(float* v) {
		if (!_initialised) {
			Vengine::fatalError("Cannot add to uninitialised similarity matrix");
		}

		if (_vectorDim == -1) {
			Vengine::fatalError("Tried to add a vector without first calling start on similarity matrix");
		}

		_start--;
		if (_start < 0) { //wrap round, just like history class
			_start = _matrixSize - 1;
		}

		memcpy(_vectorHistory.workingArray(), v, sizeof(float) * _vectorDim);
		_vectorHistory.addWorkingArrayToHistory();

		for (int i = 0; i < _vectorHistory.entries(); i++) {
			float measure = similarityMeasure(&(v[0]), _vectorHistory.get(i));
			(*getPtrToCoord(i, 0, false)) = measure; //do both sides as symmetrical around y = -x diagonal
			(*getPtrToCoord(0, i, false)) = measure;
		}

		if (!_useCorrelationWindow) {
			return;
		}

		//correlation window must be done after basic similarity matrix updated as is an average
		for (int i = 0; i < _vectorHistory.entries(); i++) {
			float correlationMeasure = correlationOverWindow(i);
			(*getPtrToCoord(i, 0, true)) = correlationMeasure; //do both sides as symmetrical around y = -x diagonal
			(*getPtrToCoord(0, i, true)) = correlationMeasure;
		}
	}


	//getters

	float get(int i, int j) { //only to be used from outside of class, use get ptr inside
		if (!_initialised) {
			Vengine::fatalError("Cannot get fromuninitialised similarity matrix");
		}

		if (i > _vectorHistory.entries() || j > _vectorHistory.entries()) {
			return 0.0f;
		}

		return *getPtrToCoord(i, j, _useCorrelationWindow);
	}

	int sideLength() const { return _matrixSize; }
	bool full() { return (_vectorHistory.entries() == _vectorHistory.totalSize()); }
	int entries() { return _vectorHistory.entries(); }
private:
	bool _initialised;

	int _matrixSize;
	int _start;

	float** _data; //store s(i,j)

	bool _useCorrelationWindow;
	int _correlationWindowSize;
	float** _dataWindowedCorrelation; //store 1/w * sum{k = [0,w - 1]]}(s(i + k, j + k))

	int _vectorDim;
	VectorHistory _vectorHistory;

	float* getPtrToCoord(int i, int j, bool getWindowedCorrelation) {
		if (getWindowedCorrelation) {
			return &_dataWindowedCorrelation[(_start + i) % _matrixSize][(_start + j) % _matrixSize];
		}
		return &_data[(_start + i) % _matrixSize][(_start + j) % _matrixSize];

	}


	float similarityMeasure(float* v1, float* v2, bool debug = false) {

		float dot = Tools::dot(v1, v2, _vectorDim);
		float n1 = Tools::L2norm(v1, _vectorDim);
		float n2 = Tools::L2norm(v2, _vectorDim);

		return dot / (n1 * n2);
	}

	float correlationOverWindow(int i) {
		int windowSizeLimitedByDataSize = std::min(_correlationWindowSize, _matrixSize - i);
		float sum = 0;
		for (int j = 0; j < windowSizeLimitedByDataSize; j++) {
			sum += *getPtrToCoord(i + j, j, false);
		}
		sum /= float(_correlationWindowSize);
		return sum;
	}
};