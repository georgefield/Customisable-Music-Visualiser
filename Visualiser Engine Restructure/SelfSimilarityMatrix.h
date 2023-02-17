#pragma once
#include "MFCCs.h"
#include "FourierTransform.h"
#include "Tools.h"
#include "History.h"

#include <glm/glm.hpp>
#include <iomanip>

#include <Vengine/MyTiming.h>


class SimilarityMatrixStructure {
public:
	SimilarityMatrixStructure(int sideLength, int correlationWindowSize = 1) :
		_sideLength(sideLength),
		_correlationWindowSize(correlationWindowSize),
		_start(_sideLength - 1), //start br and move up to tl then wrap around

		_vectorDim(-1), //test against to ensure all vectors same dimension, -1 indicates not set yet
		_vectorHistory(_sideLength),

		_data(nullptr),
		_dataWindowedCorrelation(nullptr) //init to nullptr as might not be initialised in constructor
	{

		_useCorrelationWindow = (_correlationWindowSize >= 2); // size 1 correlation window data = data, only needed for size 2 and up
		if (_correlationWindowSize > sideLength) {
			Vengine::warning("Correlation window cannot be greater than side length, defaulting to side length");
			_correlationWindowSize = sideLength;
		}

		//main data
		_data = new float* [_sideLength];
		for (int i = 0; i < _sideLength; i++) {
			_data[i] = new float[_sideLength];
			memset(_data[i], 0.0f, _sideLength * sizeof(float));
		}

		//set aside memory if using correlation window
		_dataWindowedCorrelation = new float* [_sideLength];
		for (int i = 0; i < _sideLength; i++) {
			_dataWindowedCorrelation[i] = new float[_sideLength];
			memset(_dataWindowedCorrelation[i], 0.0f, _sideLength * sizeof(float));
		}
	}

	~SimilarityMatrixStructure() {
		for (int i = 0; i < _sideLength; i++) {
			delete[] _data[i];
		}
	}

	void add(std::vector<float> v) {
		if (_vectorDim == -1) {
			_vectorDim = v.size();
			_vectorHistory.init(_vectorDim);
		}
		else if (_vectorDim != v.size()) {
			Vengine::fatalError("Tried to add a vector of different dimension to others to self similarity matrix");
		}

		_start--;
		if (_start < 0) { //wrap round, just like history class
			_start = _sideLength - 1;
		}
		_vectorHistory.add(v);

		for (int i = 0; i < _vectorHistory.entries(); i++) {
			float measure = similarityMeasure(&(v[0]), _vectorHistory.get(i));
			(*getPtrToCoord(i, 0, false)) = measure; //do both sides as symmetrical around y = -x diagonal
			(*getPtrToCoord(0, i, false)) = measure;
		}

		if (!_useCorrelationWindow) {
			return;
		}
		std::cout << "using";

		//correlation window must be done after basic similarity matrix updated as is an average
		for (int i = 0; i < _vectorHistory.entries(); i++) {
			float correlationMeasure = correlationOverWindow(i);
			(*getPtrToCoord(i, 0, true)) = correlationMeasure; //do both sides as symmetrical around y = -x diagonal
			(*getPtrToCoord(0, i, true)) = correlationMeasure;
		}
	}

	void add(float* v, int dim) //overload for if given float* not vector
	{
		std::vector<float> arrayAsVector;
		arrayAsVector.resize(dim);
		for (int i = 0; i < dim; i++) {
			arrayAsVector[i] = v[i];
		}
		add(arrayAsVector);
	}

	void clear() {
		_start = 0;
		_vectorDim = -1;
		_vectorHistory = VectorHistory(_sideLength); //replace vector history
	}

	void setCorrelationWindowSize(int windowSize) {
		_correlationWindowSize = windowSize;
		_useCorrelationWindow = (_correlationWindowSize >= 2);
	}

	//getters

	float get(int i, int j) { //only to be used from outside of class, use get ptr inside
		if (i > _vectorHistory.entries() || j > _vectorHistory.entries()) {
			return 0.0f;
		}

		return *getPtrToCoord(i, j, _useCorrelationWindow);
	}

	int sideLength() const { return _sideLength; }
	bool full() { return (_vectorHistory.entries() == _vectorHistory.totalSize()); }
	int entries() { return _vectorHistory.entries(); }
private:
	int _sideLength;
	int _start;

	float** _data; //store s(i,j)

	bool _useCorrelationWindow;
	int _correlationWindowSize;
	float** _dataWindowedCorrelation; //store 1/w * sum{k = [0,w - 1]]}(s(i + k, j + k))
	
	int _vectorDim;
	VectorHistory _vectorHistory;

	float* getPtrToCoord(int i, int j, bool getWindowedCorrelation) {
		if (getWindowedCorrelation) {
			return &_dataWindowedCorrelation[(_start + i) % _sideLength][(_start + j) % _sideLength];
		}
		return &_data[(_start + i) % _sideLength][(_start + j) % _sideLength];

	}

	float similarityMeasure(float* v1, float* v2, bool debug = false) {

		float dot = Tools::dot(v1, v2, _vectorDim);
		float n1 = Tools::L2norm(v1, _vectorDim);
		float n2 = Tools::L2norm(v2, _vectorDim);

		return dot / (n1 * n2);
	}


	float correlationOverWindow(int i) {
		int windowSizeLimitedByDataSize = std::min(_correlationWindowSize, _sideLength - i);
		float sum = 0;
		for (int j = 0; j < windowSizeLimitedByDataSize; j++) {
			sum += *getPtrToCoord(i + j, j, false);
		}
		sum /= float(_correlationWindowSize);
		return sum;
	}
};


class SelfSimilarityMatrix
{
public:

	SelfSimilarityMatrix(int size) :
		_similarityMatrix(size, 1), //default small correlation (computationally expensive)

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
	void linkToDebug();

	float getSelfSimilarityMatrixValue(int i, int j) {
		return _similarityMatrix.get(i, j);
	}

	void setCorrelationWindowSize(int windowSize) //reduces noise (smooths matrix), computationally expensive
	{
		_similarityMatrix.setCorrelationWindowSize(windowSize);
	}

	void debug();

private:
	static enum LinkedTo {
		NONE,
		DEBUG,
		MFCC,
		MelBandEnergies,
		MelSpectrogram,
		FT
	};

	SimilarityMatrixStructure _similarityMatrix;
	LinkedTo _linkedTo;

	//link ptrs
	MFCCs* _mfccsPtr;
	FourierTransform* _ftPtr;

	//debug
	int _debugTimerId;
	bool _switch;
};

