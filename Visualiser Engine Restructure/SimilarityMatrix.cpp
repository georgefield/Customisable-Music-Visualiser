#include "SimilarityMatrix.h"
#include <thread>
#include <numeric>
#include <assert.h>
#include "VisVars.h"

#define NUM_THREADS 8
 

SimilarityMatrix::SimilarityMatrix(int matWidth, int vectorDim) :
	_matrixWidth(matWidth),

	_vectorHistory(_matrixWidth),
	_vectorDim(vectorDim),

	_vectorMagnitudeHistory(_matrixWidth), //so dont have to recalculate each time

	_dataLength(_matrixWidth* _matrixWidth),
	_start(0),

	_data(nullptr),

	_similarityMeasureHistory(Vis::consts._generalHistorySize)

{
	//set up matrix--
	_data = new float[_dataLength];
	memset(_data, 0.0f, _dataLength * sizeof(float));
	//--

	_vectorHistory.init(_vectorDim);
}

SimilarityMatrix::~SimilarityMatrix()
{
	delete[] _data;

	if (_textureCreator.isCreated()) {
		_textureCreator.deleteTexture();
	}
}

void SimilarityMatrix::add(float* v, float contrastFactor) {

	_start--;
	if (_start < 0) { //wrap round, just like history class
		_start = _matrixWidth - 1;
	}

	memcpy(_vectorHistory.workingArray(), v, sizeof(float) * _vectorDim);
	_vectorHistory.addWorkingArrayToHistory();
	_vectorMagnitudeHistory.add(MyMaths::L2norm(v, _vectorDim));

	//calculate measure and add to similarity matrix--
	_data[toDataIndex(0, 0)] = 1.0f;

	for (int i = 1; i < _vectorHistory.entries(); i++) {
		//get similarity measure
		float measure = MyMaths::dot(_vectorHistory.newest(), _vectorHistory.get(i), _vectorDim);
		measure /= (_vectorMagnitudeHistory.newest() * _vectorMagnitudeHistory.get(i));

		//increase contrast
		measure = std::max(1.0f - (contrastFactor * (1.0f - measure)), 0.0f);

		_data[toDataIndex(i, 0)] = measure;
		_data[toDataIndex(0, i)] = measure;
	}
	//--

	//update texture if created
	if (_textureCreator.isCreated() && Vis::vars._computeTexture) {
		_textureCreator.updateTexture(_matrixWidth, _matrixWidth, 0, 0, _data);
	}
}

float SimilarityMatrix::getSimilarityMeasureThreaded()
{
	std::vector<std::thread> threads;
	std::vector<float> outputs(NUM_THREADS, 0.0f);

	// Determine the workload for each thread
	int chunkSize = _matrixWidth / NUM_THREADS;
	int remainder = _matrixWidth % NUM_THREADS;

	for (int i = 0; i < NUM_THREADS; i++) {
		threads.emplace_back(
			&SimilarityMatrix::getSimMeasurePart, this,
			simMeasurePartInfo(chunkSize * i, chunkSize * (i + 1) + (i == NUM_THREADS - 1 ? remainder : 0), //last chunk does remainder aswell
				outputs[i]));
	}


	for (std::thread& thread : threads) {
		thread.join();
	}

	float output = std::accumulate(outputs.begin(), outputs.end(), 0.0f);
	output /= _matrixWidth * _matrixWidth;

	_similarityMeasureHistory.add(output);
	return output;
}

float SimilarityMatrix::getSimilarityMeasure()
{
	float sum = 0;
	for (int i = 0; i < _matrixWidth; i++) {
		for (int j = 0; j < _matrixWidth; j++) {
			sum += _data[toDataIndex(i, j)] * checkerboardKernel(i, j);
		}
	}
	sum /= _matrixWidth * _matrixWidth;

	_similarityMeasureHistory.add(sum);
	return sum;
}

// --- PRIVATE ---

inline float SimilarityMatrix::checkerboardKernel(int i, int j) {
	float middle = (_matrixWidth - 1) / 2.0f;
	int sign = 1;
	if ((i < middle && j > middle) || (i > middle && j < middle)) {
		sign = -1;
	}

	//sqrt(2) * middle is max distance from middle - distance from middle * sign 
	return ((1.414 * middle) - sqrt((i - middle) * (i - middle) + (j - middle) * (j - middle))) * sign / (1.414 * middle);
}

void SimilarityMatrix::getSimMeasurePart(const simMeasurePartInfo& info) {
	float sum = 0;
	for (int i = 0; i < _matrixWidth; i++) {
		for (int j = info.jStart; j < info.jEnd; j++) {
			sum += _data[toDataIndex(i,j)] * checkerboardKernel(i, j);
		}
	}
	sum /= _matrixWidth * _matrixWidth;
	info.out = sum;
}