#pragma once
#include "Master.h"
#include "FFTW.h"

enum class FourierTransformType {
	STANDARD,
	TIME_CONVOLVED,
	SMOOTHED
};

class FFTs {
public:
	FFTs(int historySize, int N) :
		_fftHistoryPtr(nullptr),
		_fftwAPI(historySize, N),
		_N(N),
		_numHarmonics(N * 0.5 + 1),
		_timeConvolvedFftHistory(historySize),
		_smoothedFftHistory(historySize)
	{
	}

	~FFTs() {
		//delete memory allocated for fft histories
		for (int i = 0; i < _timeConvolvedFftHistory.totalSize(); i++) {
			delete[] _timeConvolvedFftHistory.get(i);
		}
		for (int i = 0; i < _smoothedFftHistory.totalSize(); i++) {
			delete[] _smoothedFftHistory.get(i);
		}
	}

	void init(Master* master);

	//used for auto calculating fft
	void calculateFft(FourierTransformType type);
	History<float*>* getFftHistory(FourierTransformType type);

	//keep public in case need to be able to specify details--
	void calculateStandardFft();

	void calculateTimeConvolvedFft(int previousXtransforms = 7, Kernel kernel = LINEAR_PYRAMID);

	void calculateSmoothedFft(bool useTimeConvolvedFft = false, float attack = 0.15, float release = 0.5, float maxAccelerationPerSecond = 0.5);
	//--
	
	void convolveOverHarmonics(float* in, float* out, int windowSize, Kernel kernel);

	int _N;
	int _numHarmonics;

private:
	Master* _m;


	FFTW _fftwAPI;

	History<float*>* _fftHistoryPtr;
	History<float*> _timeConvolvedFftHistory;
	History<float*> _smoothedFftHistory;

	float* _smoothedFftDot;

	int _sampleFftLastCalculated;
};

