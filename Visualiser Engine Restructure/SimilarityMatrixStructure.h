#pragma once
#include "VectorHistory.h"
#include <Vengine/MyErrors.h>
#include "Tools.h"
#include "DataTextureCreator.h"
#include "SPvars.h"

class SimilarityMatrixStructure {
public:
	SimilarityMatrixStructure(int matrixSize) :
		_vectorHistory(matrixSize),
		_vectorMagnitudeHistory(matrixSize),
		_matrixSize(matrixSize),
		_dataLength(matrixSize * matrixSize),
		_usingOrderedData(false),

		_data(nullptr),
		_dataWindowedCorrelation(nullptr), //init to nullptr as might not be initialised in constructor
		_orderedData(nullptr),

		_initialised(false)
	{
	}


	~SimilarityMatrixStructure() {
		if (_initialised) {
			delete[] _data;
			delete[] _dataWindowedCorrelation;
		}
		if (_usingOrderedData) {
			delete[] _orderedData;
		}
		if (_textureCreator.isCreated()) {
			_textureCreator.deleteTexture();
		}
	}

	void init(int correlationWindowSize = 1) {
		_initialised = true;

		_start = 0;

		//set up matrix--
		_data = new float[_dataLength];
		memset(_data, 0.0f, _dataLength * sizeof(float));
		//--

		//always set aside memory for correlation window matrix as just makes life easier--
		setCorrelationWindowSize(correlationWindowSize);

		_dataWindowedCorrelation = new float[_dataLength];
		memset(_dataWindowedCorrelation, 0.0f, _dataLength * sizeof(float));
		//--

		_vectorHistory.init(_vectorDim);
	}

	void reInit(int correlationWindowSize = 1) {
		_start = 0;

		_vectorHistory.clear();
		_vectorMagnitudeHistory.clear();

		//reset texture if created
		if (_textureCreator.isCreated()) {
			deleteTexture();
			createTexture(SP::vars._fastSimilarityMatrixTexture);
		}

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

	void add(float* v, float contrastFactor) {
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
		_vectorMagnitudeHistory.add(Tools::L2norm(v, _vectorDim));

		//calculate measure and add to similarity matrix--
		setCoord(0, 0, false, 1.0f);

		for (int i = 1; i < _vectorHistory.entries(); i++) {
			//similarity measure
			float measure = Tools::dot(_vectorHistory.newest(), _vectorHistory.get(i), _vectorDim);
			measure /= (_vectorMagnitudeHistory.newest() * _vectorMagnitudeHistory.get(i));

			//increase contrast
			measure = 1.0f - (contrastFactor * (1.0f - measure));

			setCoord(i, 0, false, measure);
			setCoord(0, i, false, measure);
		}
		//--

		//calculate correlation window matrix value if applicable--
		if (_useCorrelationWindow) {

			setCoord(0, 0, true, 1.0f);

			//correlation window must be done after basic similarity matrix updated as is an average
			for (int i = 0; i < _vectorHistory.entries(); i++) {
				float correlationMeasure = correlationOverWindow(i);
				setCoord(i, 0, true, correlationMeasure);
				setCoord(0, i, true, correlationMeasure);
			}
		}
		//--

		if (_textureCreator.isCreated() && SP::vars._computeTexture) {
			updateTexture(); 
		}
	}

	void createTexture(bool fast) {
		if (_textureCreator.isCreated()) {
			_textureCreator.deleteTexture();
		}

		//if using slow updater that creates easier to work with texture--
		_usingOrderedData = !fast;
		if (_usingOrderedData) {
			if (_orderedData == nullptr) {
				_orderedData = new float[_dataLength];
			}
			SLOWgetDataInCorrectMemoryOrder(_orderedData);
			_textureCreator.createTexture(_matrixSize, _matrixSize, _orderedData);
			return;
		}
		//--

		if (!_useCorrelationWindow) {
			_textureCreator.createTexture(_matrixSize, _matrixSize, _data);
			return;
		}

	}
	void deleteTexture() {
		_textureCreator.deleteTexture();
	}

	//getters

	float get(int i, int j) { //only to be used from outside of class, use get ptr inside
		if (!_initialised) {
			Vengine::fatalError("Cannot get from uninitialised similarity matrix");
		}

		if (i > _vectorHistory.entries() || j > _vectorHistory.entries()) {
			return 0.0f;
		}

		if (_useCorrelationWindow) {
			return _dataWindowedCorrelation[((_start + i) % _matrixSize) * _matrixSize + ((_start + j) % _matrixSize)];
		}
		return _data[((_start + i) % _matrixSize) * _matrixSize + ((_start + j) % _matrixSize)];
	}

	Vengine::GLtexture getMatrixTexture() {
		return _textureCreator.getTexture();
	}

	int matrixSize() const { return _matrixSize; }
	bool full() { return (_vectorHistory.entries() == _vectorHistory.totalSize()); }
	int entries() { return _vectorHistory.entries(); }
	int textureStartIndex() {
		if (_usingOrderedData) { return 0; }
		return _start;
	}
	bool isTextureCreated() { return _textureCreator.isCreated(); }
private:
	bool _initialised;

	int _matrixSize;
	int _dataLength;
	int _start;

	float* _data; //store s(i,j)
	DataTextureCreator _textureCreator;

	bool _useCorrelationWindow;
	int _correlationWindowSize;
	float* _dataWindowedCorrelation; //store 1/w * sum{k = [0,w - 1]]}(s(i + k, j + k))

	float* _orderedData; //only used when wanting slow texture updating
	bool _usingOrderedData;

	int _vectorDim;
	VectorHistory<float> _vectorHistory;
	History<float> _vectorMagnitudeHistory;


	void setCoord(int i, int j, bool windowedCorrelation, float value) {
		if (windowedCorrelation) {
			_dataWindowedCorrelation[((_start + i) % _matrixSize) * _matrixSize + ((_start + j) % _matrixSize)] = value;
			return;
		}
		_data[((_start + i) % _matrixSize) * _matrixSize + ((_start + j) % _matrixSize)] = value;
	}

	float correlationOverWindow(int i) {
		int windowSizeLimitedByDataSize = std::min(_correlationWindowSize, _matrixSize - i);
		float sum = 0;
		for (int j = 0; j < windowSizeLimitedByDataSize; j++) {
			sum += _data[((_start + i + j) % _matrixSize) * _matrixSize + ((_start + j) % _matrixSize)];
		}
		sum /= float(_correlationWindowSize);
		return sum;
	}

	void updateTexture() {
		float* data = _data;

		if (_usingOrderedData) {
			SLOWgetDataInCorrectMemoryOrder(_orderedData);
			data = _orderedData;
		}
		else if (_useCorrelationWindow) {
			data = _dataWindowedCorrelation;
		}

		_textureCreator.updateTexture(_matrixSize, _matrixSize, 0, 0, data);
	}

	void SLOWgetDataInCorrectMemoryOrder(float* out) {
		for (int i = 0; i < _matrixSize; i++) {
			for (int j = 0; j < i; j++) {
				out[i * _matrixSize + j] = get(i, j);
				out[j * _matrixSize + i] = out[i * _matrixSize + j];
			}
			out[i * _matrixSize + i] = get(i, i);
		}
	}
};