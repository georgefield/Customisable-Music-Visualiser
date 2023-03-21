#include "Master.h"
#include "AudioManager.h"
#include <Vengine/MyErrors.h>
#include <cmath>
#include <functional>

#include "VisualiserShaderManager.h"
#include "SPvars.h"

Master::Master() :
	_audioData(nullptr),

	_fftwAPI(SP::consts._STFTsamples),
	_fftHistory(7),//store 7 previous fourier transforms

	_sampleFftLastCalculated(-1),
	_sampleRate(0),

	_peakAmplitude(0),
	_sampleOfLastPeak(0),
	_energy(0),
	_energyHistory(SP::consts._generalHistorySize)
{
}

Master::~Master()
{
	if (_useSetters) {
		removeUpdaters();
	}
}

void Master::init(float* audioData, int sampleRate, bool useSetters)
{
	_fftHistory.init(_fftwAPI.numHarmonics());

	_audioData = audioData;
	_sampleRate = sampleRate;

	_useSetters = useSetters;
	if (_useSetters) {
		setUpdaters();
	}
}

void Master::reInit(float* audioData, int sampleRate) {

	_audioData = audioData;
	_sampleRate = sampleRate;

	_previousSample = -1;
	_sampleFftLastCalculated = -1;
}

void Master::beginCalculations(int currentSample) {

	assert(_sampleRate > 0);
	_currentSample = currentSample;

	if (_currentSample == _previousSample) {
		Vengine::warning("No change in sample between begin calculation calls");
	}
}


float hanningWindow(float frac){ //reduces noise
	return -0.5f * cosf(2.0f * 3.1415926f * frac) + 0.5f; //hanning function, increases min detectable frequency by 2, [worst case 24hz]
}

void Master::calculateFourierTransform() {

	//dont calculate if at end
	if (_currentSample + SP::consts._STFTsamples >= AudioManager::getNumSamples()) {
		return;
	}

	//make sure not called twice on same frame
	if (_sampleFftLastCalculated == _currentSample) {
		return;
	}
	_sampleFftLastCalculated = _currentSample;

	_fftwAPI.calculate(_audioData, _currentSample, _fftHistory.workingArray(), SP::vars._masterFTgain, hanningWindow); //use fftw api to calculate fft
	_fftHistory.addWorkingArrayToHistory();
	//updates _fftHistory ^^^
}
 
//accounts for hanning window so calculations from samples and transforms are equivalent
void Master::calculateEnergy()
{
	//dont calculate if at end
	if (_currentSample + SP::consts._STFTsamples >= AudioManager::getNumSamples()) {
		return;
	}

	_energy = 0;
	float hanningWindowFactor = 0;

	for (int i = _currentSample; i < _currentSample + SP::consts._STFTsamples; i++) {
		hanningWindowFactor = hanningWindow(float(i - _currentSample) / float(SP::consts._STFTsamples));
		_energy += _audioData[i] * _audioData[i] * hanningWindowFactor * hanningWindowFactor;
	}
	_energy /= SP::consts._STFTsamples;
	_RMS = sqrt(_energy);
	_energyHistory.add(_energy);
}

void Master::calculatePeakAmplitude()
{
	//dont calculate if at end
	if (_currentSample >= AudioManager::getNumSamples()) {
		return;
	}

	if (_currentSample - _sampleOfLastPeak > _sampleRate * 0.1f) { //after waiting 0.1 seconds at peak
		_peakAmplitude *= expf(-30.0f / SP::vars._desiredCPS); //0->-30db in 1 second
	}
	
	for (int i = _previousSample; i < _currentSample; i++) {
		if (fabsf(_audioData[i]) > _peakAmplitude) {
			_peakAmplitude = fabsf(_audioData[i]);
			_sampleOfLastPeak = i;
		}
	}

	_peakAmplitude = std::min(10.0f, _peakAmplitude);

	_peakAmplitudeDb = log(_peakAmplitude);
}

void Master::audioIsPaused()
{
	_peakAmplitude *= expf(-30.0f / SP::vars._desiredCPS); //0->-30db in 1 second
	_peakAmplitudeDb = log(_peakAmplitude);
}

void Master::endCalculations() {

	_previousSample = _currentSample;
}


//*** helper functions ***

//used to create a new history which is smoothed
//better than a rolling average as big individual values are first added with a small constant (for hill shaped kernels atleast)
float Master::sumOfConvolutionOfHistory(History<float>* history, int entries, Kernel kernel) {

	if (entries == 0) entries = history->totalSize();//default convolve all history

	float conv = 0;
	for (int i = 0; i < entries; i++) {
		conv += history->get(i) * Kernels::apply(kernel, i, entries); //integral of the multiplication = dot product   (in discrete space)
	}
	return conv;
}

//simple getters--
float* Master::getBaseFftOutput() { return _fftHistory.newest(); }
MyComplex* Master::getBaseFftComplexOutput() { return _fftwAPI.getComplexCoeffsHistory(_currentSample)->newest(); }
VectorHistory<MyComplex>* Master::getBaseFftComplexOutputHistory() { return _fftwAPI.getComplexCoeffsHistory(_currentSample); }
int Master::getBaseFftNumHarmonics() { return _fftHistory.numHarmonics(); }
float Master::getPeakAmplitude(){ return _peakAmplitude; }
float Master::getPeakAmplitudeDb() { return _peakAmplitudeDb; }
float Master::getEnergy() { return _energy; }
float Master::getRMS() { return _RMS; }
//--

void Master::setUpdaters()
{
	std::function<int()> masterNumHarmonicsUpdaterFunction = std::bind(&Master::getBaseFftNumHarmonics, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_FTmaster_size", masterNumHarmonicsUpdaterFunction);

	std::function<float()> masterEnergyUpdaterFunction = std::bind(&Master::getEnergy, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_energy", masterEnergyUpdaterFunction);

	std::function<float()> masterRMSupdaterFunction = std::bind(&Master::getRMS, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_RMS", masterRMSupdaterFunction);

	std::function<float()> peakAmplitudeUpdaterFunction = std::bind(&Master::getPeakAmplitude, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_peakAmplitude", peakAmplitudeUpdaterFunction);

	std::function<float()> peakAmplitudeDbUpdaterFunction = std::bind(&Master::getPeakAmplitudeDb, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_peakAmplitudeDb", peakAmplitudeDbUpdaterFunction);

	std::function<float* ()> harmonicValuesUpdaterFunction = std::bind(&Master::getBaseFftOutput, this);
	VisualiserShaderManager::SSBOs::setSSBOupdater("vis_FTmaster_harmonics", harmonicValuesUpdaterFunction, _fftHistory.numHarmonics());
}

void Master::removeUpdaters()
{
	//todo
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_FTmaster_size");
}
