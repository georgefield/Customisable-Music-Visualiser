#include "FourierTransform.h"
#include "VisualiserShaderManager.h"
#include "SignalProcessingManager.h"

FourierTransform::FourierTransform(int historySize, float cutOffLow, float cutOffHigh, float cutoffSmoothFrac) :

	_working1(historySize),
	_working2(historySize),

	_initialised(false),

	_numHarmonics(-1),

	_current(nullptr),
	_next(nullptr),

	_energyOfFt(SP::consts._generalHistorySize)
{
	_FTinfo.cutoffLow = cutOffLow;
	_FTinfo.cutoffHigh = cutOffHigh;
	_FTinfo.isMinCutoffLow = cutOffLow < 0;
	_FTinfo.isMaxCutoffHigh = cutOffHigh < 0;
	_FTinfo.cutoffSmoothFrac = cutoffSmoothFrac;
	_FTinfo.historySize = historySize;

	_maxLowResOutputSize = 50;
}

FourierTransform::~FourierTransform()
{
	if (_initialised) {
		delete[] _smoothedFt;
		delete[] _smoothedFtDot;
		delete[] _lowResOutputLogScale;

		if (_useSetters) {
			removeUpdaters();
			std::cout << "Removing updaters for ft " << _FTinfo.id << std::endl;
		}
	}
}


void FourierTransform::init(Master* master, int FTid)
{
	if (_initialised) {
		Vengine::fatalError("Double initialisation of fourier transform");
	}

	_initialised = true;
	_FTinfo.id = FTid;
	_useSetters = (FTid >= 0); //-1 for FT that does not output to gpu for use in shaders
	_m = master;

	//sanitise and set cutoffs--
	if (_FTinfo.cutoffHigh > _m->nyquist())
		_FTinfo.isMaxCutoffHigh = true;

	if (_FTinfo.isMinCutoffLow)
		_FTinfo.cutoffLow = 0.0f;

	if (_FTinfo.isMaxCutoffHigh)
		_FTinfo.cutoffHigh = _m->nyquist();
	//--

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
	_energyOfFt.init(_m, _FTinfo.id);

	if (_useSetters) {
		setUpdaters();
	}
}

void FourierTransform::reInit()
{
	if (!_initialised) {
		Vengine::fatalError("Cannot re-init unitialised fourier transform");
	}

	//make sure max cutoffs are adjusted for possible changed sample rate--
	if (_FTinfo.isMinCutoffLow)
		_FTinfo.cutoffLow = 0.0f;

	if (_FTinfo.isMaxCutoffHigh)
		_FTinfo.cutoffHigh = _m->nyquist();
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

void FourierTransform::calculateNext()
{
	beginCalculation();
	if (_FTinfo.applyFirst != NO_FUNCTION) {
		applyFunction(_FTinfo.applyFirst);
	}
	if (_FTinfo.applySecond != NO_FUNCTION) {
		applyFunction(_FTinfo.applySecond);
	}
	if (_FTinfo.applyThird != NO_FUNCTION) {
		applyFunction(_FTinfo.applyThird);
	}
	endCalculation();
}


void FourierTransform::setSmoothEffect(int firstSecondOrThird, FunctionType function)
{
	assert(firstSecondOrThird >= 0 && firstSecondOrThird <= 2);

	if (firstSecondOrThird == 0) {
		_FTinfo.applyFirst = function;
		return;
	}
	if (firstSecondOrThird == 1) {
		_FTinfo.applySecond = function;
		return;
	}
	if (firstSecondOrThird == 2) {
		_FTinfo.applyThird = function;
		return;
	}
}


void FourierTransform::addSmoothEffect(FunctionType function)
{
	if (_FTinfo.applyFirst == NO_FUNCTION) {
		_FTinfo.applyFirst = function;
		return;
	}
	if (_FTinfo.applySecond == NO_FUNCTION) {
		_FTinfo.applySecond = function;
		return;
	}
	if (_FTinfo.applyThird == NO_FUNCTION) {
		_FTinfo.applyThird = function;
		return;
	}
}

void FourierTransform::removeSmoothEffect()
{
	if (_FTinfo.applyThird != NO_FUNCTION) {
		_FTinfo.applyThird = NO_FUNCTION;
		return;
	}
	if (_FTinfo.applySecond != NO_FUNCTION) {
		_FTinfo.applySecond = NO_FUNCTION;
		return;
	}
	if (_FTinfo.applyFirst != NO_FUNCTION) {
		_FTinfo.applyFirst = NO_FUNCTION;
		return;
	}

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

	//energy calculated from FT before any smoothing otherwise would be inaccurate
	_energyOfFt.calculateNext(_current->workingArray(), _current->numHarmonics());
}

void FourierTransform::endCalculation() {
	_current->addWorkingArrayToHistory();

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
	if (_FTinfo.freqWindowSize == 1) {
		return;
	}

	memset(out, 0.0f, sizeof(float) * _numHarmonics);

	for (int i = 0; i < _numHarmonics; i++) {
		int jStart = std::max(i - (_FTinfo.freqWindowSize / 2), 0);
		for (int j = jStart; j < std::min(i + (_FTinfo.freqWindowSize / 2), _numHarmonics); j++) {
			out[i] += in[j] * Kernels::apply(LINEAR_PYRAMID, j - jStart, _FTinfo.freqWindowSize);
		}
	}
}

void FourierTransform::applySmoothing(float* in, float* out) {
	if (_m->_previousSample == -1) {
		return; //need 2 samples
	}

	//calculate smoothed ft from 'in' ft
	float timeSinceLastCalculation = float(_m->_currentSample - _m->_previousSample) / float(_m->_sampleRate);
	float maxAcceleration = _FTinfo.maxAccelerationPerSecond * timeSinceLastCalculation;

	for (int i = 0; i < _numHarmonics; i++) {
		if (in[i] > _smoothedFt[i]) {
			float change = std::fmin(timeSinceLastCalculation / _FTinfo.attack, in[i] - _smoothedFt[i]);
			change = std::fmin(_smoothedFtDot[i] + maxAcceleration, change); //clamp change to be within max acceleration of last change
			_smoothedFtDot[i] = change;
		}
		else {
			float change = -std::fmin(timeSinceLastCalculation / _FTinfo.release, _smoothedFt[i] - in[i]);
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
	if (_FTinfo.timeWindowSize > _FTinfo.historySize) {
		Vengine::warning("Requested to convolve over more frames than exists stored fft data, defaulting to over all stored data");
		_FTinfo.timeWindowSize = _FTinfo.historySize;
	}

	memset(out, 0.0f, sizeof(float) * _numHarmonics);

	for (int i = 0; i < _FTinfo.timeWindowSize; i++) {
		for (int j = 0; j < _numHarmonics; j++) {
			out[j] += in->get(i)[j] * Kernels::apply(LINEAR_PYRAMID, i, _FTinfo.timeWindowSize); //overwrite oldest
		}
	}
}


void FourierTransform::setFourierTransformFrequencyInfo()
{
	//calculate cutoff points and sanitise input--
	float fourierLowFrac = _FTinfo.cutoffLow / _m->nyquist(); //as nyquist max freq
	float fourierHighFrac = _FTinfo.cutoffHigh / _m->nyquist();
	if (fourierHighFrac > 1) {
		Vengine::warning("Cutoff high > nyquist, setting equal to nyquist");
		_FTinfo.cutoffHigh = _m->nyquist();
		fourierHighFrac = 1;
	}
	if (fourierLowFrac < 0) {
		Vengine::warning("Cutoff low < 0, setting equal to 0");
		_FTinfo.cutoffLow = 0;
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

float FourierTransform::smoothCutoff(int i)
{
	if (_FTinfo.cutoffSmoothFrac == 0.0f) {
		return 1.0f; //no smoothing
	}

	float distanceFromCutoffFrac = float(std::min(i - _harmonicLow, _harmonicHigh - i)) / float(_numHarmonics);

	return std::min((2.0f * distanceFromCutoffFrac) / _FTinfo.cutoffSmoothFrac, 1.0f); //1.0f => pyramid band, 0.5f => trapezium with top side half of bottom, 0.1f => trapezium top side 9/10 of bottom
}

void FourierTransform::setUpdaters()
{
	std::string uniformPrefix = "vis_FT" + std::to_string(_FTinfo.id) + "_";

	//energy setters init in energy class

	std::function<int()> numHarmonicsSetterFunction = std::bind(& FourierTransform::getNumHarmonics, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater(uniformPrefix + "size", numHarmonicsSetterFunction);

	std::function<float* ()> harmonicValueSetterFunction = std::bind(&FourierTransform::getOutput, this);
	VisualiserShaderManager::SSBOs::setSSBOupdater(uniformPrefix + "harmonics", harmonicValueSetterFunction, _numHarmonics);
}

void FourierTransform::removeUpdaters()
{
	std::string uniformPrefix = "vis_FT" + std::to_string(_FTinfo.id) + "_";

	//energy setters deleted in energy class

	VisualiserShaderManager::Uniforms::removeUniformUpdater(uniformPrefix + "size");
	VisualiserShaderManager::SSBOs::removeSSBOupdater(uniformPrefix + "harmonics");
}
