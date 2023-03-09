#include "FourierTransform.h"
#include "VisualiserShaderManager.h"
#include "SignalProcessingManager.h"

FourierTransform::FourierTransform(int historySize, float cutOffLow, float cutOffHigh, float cutoffSmoothFrac) :

	_cutOffLow(cutOffLow),
	_cutOffHigh(cutOffHigh),
	_minCutoffLow(cutOffLow < 0),
	_maxCutoffHigh(cutOffHigh < 0),

	_cutoffSmoothFrac(cutoffSmoothFrac),

	_working1(historySize),
	_working2(historySize),

	_historySize(historySize),
	_initialised(false),

	_numHarmonics(-1),

	_current(nullptr),
	_next(nullptr),

	_energyOfFt(SPvars::Const::_generalHistorySize)
{
	_maxLowResOutputSize = 50;
}

FourierTransform::~FourierTransform()
{
	if (_initialised) {
		delete[] _smoothedFt;
		delete[] _smoothedFtDot;
		delete[] _lowResOutputLogScale;

		if (_useSetters) {
			deleteSetters();
		}
	}
}


void FourierTransform::init(Master* master, std::string name)
{
	if (_initialised) {
		Vengine::fatalError("Double initialisation of fourier transform");
	}

	_initialised = true;
	_nameOfFT = name;
	_useSetters = (name != ""); //empty name => not a front facing ft, so dont init setters for uniforms & ssbos
	_m = master;

	//sanitise and set cutoffs--
	if (_cutOffHigh > _m->nyquist())
		_maxCutoffHigh = true;

	if (_minCutoffLow)
		_cutOffLow = 0.0f;

	if (_maxCutoffHigh)
		_cutOffHigh = _m->nyquist();
	//--

	initDefaultVars();

	setFourierTransformFrequencyInfo();

	//needed for smoothing
	_smoothedFt = new float[_numHarmonics];
	memset(_smoothedFt, 0.0f, sizeof(float) * _numHarmonics);

	_smoothedFtDot = new float[_numHarmonics];
	memset(_smoothedFtDot, 0.0f, sizeof(float) * _numHarmonics);

	//low res spectrum vars
	_lowResOutputSize = std::min(_maxLowResOutputSize, _numHarmonics);
	_lowResOutputLogScale = new float[_lowResOutputSize];
	memset(_lowResOutputLogScale, 0.0f, _lowResOutputSize * sizeof(float));

	//init working historys
	_working1.init(_numHarmonics);
	_working2.init(_numHarmonics);

	_current = &(_working1);
	_next = &(_working2);

	//init energy of ft
	_energyOfFt.init(_m, _nameOfFT);

	if (_useSetters) {
		initSetters();
	}
}

void FourierTransform::reInit()
{
	if (!_initialised) {
		Vengine::fatalError("Cannot re-init unitialised fourier transform");
	}

	//make sure max cutoffs are adjusted for possible changed sample rate--
	if (_minCutoffLow)
		_cutOffLow = 0.0f;

	if (_maxCutoffHigh)
		_cutOffHigh = _m->nyquist();
	//--

	setFourierTransformFrequencyInfo();

	//delete all arrays with size _numHarmonics (as might have changed size if STFT window changed)
	delete[] _smoothedFt;
	delete[] _smoothedFtDot;
	delete[] _lowResOutputLogScale;

	//recreate and initialise arrays
	_smoothedFt = new float[_numHarmonics];
	memset(_smoothedFt, 0.0f, sizeof(float) * _numHarmonics);

	_smoothedFtDot = new float[_numHarmonics];
	memset(_smoothedFtDot, 0.0f, sizeof(float) * _numHarmonics);

	_lowResOutputSize = std::min(_maxLowResOutputSize, _numHarmonics);
	_lowResOutputLogScale = new float[_lowResOutputSize];
	memset(_lowResOutputLogScale, 0.0f, _lowResOutputSize * sizeof(float));

	//reinit fourier transform history to have correct # harmonics
	_working1.changeNumHarmonics(_numHarmonics);
	_working2.changeNumHarmonics(_numHarmonics);

	_current = &(_working1);
	_next = &(_working2);

	_energyOfFt.reInit();
}

void FourierTransform::beginCalculation()
{
	if (!_initialised) {
		Vengine::fatalError("calculate called before initialise in fourier transform class");
	}

	//reset which is current and next at start of calculations
	_current = &(_working1);
	_next = &(_working2);

	//get the fourier transform that is cutoff at between selected frequencies
	int index = 0;
	for (int i = _harmonicLow; i <= _harmonicHigh; i++) {
		_current->workingArray()[index] = _m->_fftHistory.newest()[i] * smoothCutoff(i);
		index++;
	}
}

void FourierTransform::endCalculation() {
	_current->addWorkingArrayToHistory(_m->_currentSample);

	//energy calculated from FT
	_energyOfFt.calculateNext(getOutput(), _current->numHarmonics());

	//calculate low res log scale output
	memset(_lowResOutputLogScale, 0.0f, _lowResOutputSize * sizeof(float));

	float numParentHarmonics = _m->_fftHistory.numHarmonics();

	float logHzFracLow = logf(_harmonicLow + 1.0f) / logf(numParentHarmonics + 1.0f);
	float logHzFracHigh = logf(_harmonicHigh + 1.0f) / logf(numParentHarmonics + 1.0f);

	int previousOutputIndex = -1;
	for (int i = _harmonicLow; i <= _harmonicHigh; i++) {
		float logHzFrac = logf(i + 1.0f) / logf(numParentHarmonics + 1.0f);

		float outputFrac = (logHzFrac - logHzFracLow) / (logHzFracHigh - logHzFracLow);
		int outputIndex = int(outputFrac * _lowResOutputSize);

		//some frequency convolution
		_lowResOutputLogScale[outputIndex] += _current->newest()[i - _harmonicLow];

		//in case skipped
		for (int j = previousOutputIndex + 1; j < outputIndex; j++) {
			_lowResOutputLogScale[j] = _lowResOutputLogScale[outputIndex];
		}
		previousOutputIndex = outputIndex;
	}
}


void FourierTransform::applyFunction(FunctionType type)
{
	if (type == TIME_CONVOLVE) {
		applyFreqConvolving(_current->workingArray(), _next->workingArray());
	}
	if (type == SMOOTH) {
		applySmoothing(_current->workingArray(), _next->workingArray());
	}
	if (type == FREQUENCY_CONVOLVE) {
		applyFreqConvolving(_current->workingArray(), _next->workingArray());
	}


	//swap current and next
	auto tmp = _current;
	_current = _next;
	_next = tmp;
}

void FourierTransform::applyFunctions(FunctionType* args, int numArgs)
{
	for (int i = 0; i < numArgs; i++) {
		applyFunction(args[i]);
	}
}


//private


void FourierTransform::applyFreqConvolving(float* in, float* out)
{
	if (_windowSize == 1) {
		return;
	}

	memset(out, 0.0f, sizeof(float) * _numHarmonics);

	for (int i = 0; i < _numHarmonics; i++) {
		int jStart = std::max(i - (_windowSize / 2), 0);
		for (int j = jStart; j < std::min(i + (_windowSize / 2), _numHarmonics); j++) {
			out[i] += in[j] * Kernels::apply(_freqKernel, j - jStart, _windowSize);
		}
	}
}

void FourierTransform::applySmoothing(float* in, float* out) {
	if (_m->_previousSample == -1) {
		return; //need 2 samples
	}

	//calculate smoothed ft from 'in' ft
	float timeSinceLastCalculation = float(_m->_currentSample - _m->_previousSample) / float(_m->_sampleRate);
	float maxAcceleration = _maxAccelerationPerSecond * timeSinceLastCalculation;

	for (int i = 0; i < _numHarmonics; i++) {
		if (in[i] > _smoothedFt[i]) {
			float change = std::fmin(timeSinceLastCalculation / _attack, in[i] - _smoothedFt[i]);
			change = std::fmin(_smoothedFtDot[i] + maxAcceleration, change); //clamp change to be within max acceleration of last change
			_smoothedFtDot[i] = change;
		}
		else {
			float change = -std::fmin(timeSinceLastCalculation / _release, _smoothedFt[i] - in[i]);
			change = std::fmax(_smoothedFtDot[i] - maxAcceleration, change);
			_smoothedFtDot[i] = change;
		}
		_smoothedFt[i] += _smoothedFtDot[i];
		if (_smoothedFt[i] < 0) {
			_smoothedFt[i] = 0;
		}
	}

	//set out
	memcpy(out, _smoothedFt, sizeof(float) * _numHarmonics);
}

void FourierTransform::applyTimeConvolving(FourierTransformHistory* in, float* out) {

	//sanity check input
	if (_previousXtransforms > _historySize) {
		Vengine::warning("Requested to convolve over more frames than exists stored fft data, defaulting to over all stored data");
		_previousXtransforms = _historySize;
	}

	memset(out, 0.0f, sizeof(float) * _numHarmonics);

	for (int i = 0; i < _previousXtransforms; i++) {
		for (int j = 0; j < _numHarmonics; j++) {
			out[j] += in->get(i)[j] * Kernels::apply(_timeKernel, i, _previousXtransforms); //overwrite oldest
		}
	}
}


void FourierTransform::setFourierTransformFrequencyInfo()
{
	//calculate cutoff points and sanitise input--
	float fourierLowFrac = _cutOffLow / _m->nyquist(); //as nyquist max freq
	float fourierHighFrac = _cutOffHigh / _m->nyquist();
	if (fourierHighFrac > 1) {
		Vengine::warning("Cutoff high > nyquist, setting equal to nyquist");
		_cutOffHigh = _m->nyquist();
		fourierHighFrac = 1;
	}
	if (fourierLowFrac < 0) {
		Vengine::warning("Cutoff low < 0, setting equal to 0");
		_cutOffLow = 0;
		fourierLowFrac = 0;
	}
	//--

	_harmonicLow = floorf(fourierLowFrac * float(_m->_fftHistory.numHarmonics()));
	_harmonicHigh = ceilf(fourierHighFrac * float(_m->_fftHistory.numHarmonics()));

	//check values work
	if (_harmonicHigh >= _m->_fftHistory.numHarmonics()) {
		_harmonicHigh = _m->_fftHistory.numHarmonics() - 1;
	}
	if (_harmonicLow < 0) {
		_harmonicLow = 0;
	}

	//calculate num harmonics
	_numHarmonics = (_harmonicHigh - _harmonicLow) + 1;

}

void FourierTransform::initDefaultVars()
{
	//default freq convolve vars
	_windowSize = 5;
	_freqKernel = LINEAR_PYRAMID;

	//default smooth vars
	_attack = 0.15;
	_release = 0.5;
	_maxAccelerationPerSecond = 0.5;

	//default time convolve vars
	_previousXtransforms = std::min(_historySize, 7);
	_timeKernel = LINEAR_PYRAMID;
}

float FourierTransform::smoothCutoff(int i)
{
	if (_cutoffSmoothFrac == 0.0f) {
		return 1.0f; //no smoothing
	}

	float distanceFromCutoffFrac = float(std::min(i - _harmonicLow, _harmonicHigh - i)) / float(_numHarmonics);

	return std::min((2.0f * distanceFromCutoffFrac) / _cutoffSmoothFrac, 1.0f); //1.0f => pyramid band, 0.5f => trapezium with top side half of bottom, 0.1f => trapezium top side 9/10 of bottom
}

void FourierTransform::initSetters()
{
	//energy setters init in energy class
	std::function<int()> numHarmonicsSetterFunction = std::bind(& FourierTransform::getNumHarmonics, this);
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter(_nameOfFT + " #freq", numHarmonicsSetterFunction);

	std::function<float* ()> harmonicValueSetterFunction = std::bind(&FourierTransform::getOutput, this);
	VisualiserShaderManager::SSBOs::addPossibleSSBOSetter(_nameOfFT + " freq values", harmonicValueSetterFunction, _numHarmonics);
}

void FourierTransform::deleteSetters()
{
	//energy setters deleted in energy class

	VisualiserShaderManager::Uniforms::deletePossibleUniformSetter(_nameOfFT + " #freq");
	VisualiserShaderManager::SSBOs::deleteSSBOsetter(_nameOfFT + " freq values");
}
