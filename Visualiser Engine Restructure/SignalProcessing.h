#pragma once
#include "Master.h"
#include "NoteOnset.h"
#include "Energy.hpp"
#include "RMS.hpp"
#include "TempoDetection.h"

#include <GL/glew.h>

class SignalProcessing {
public:
	SignalProcessing() :
		_master(4096, 2049),
		_rms(2049),
		_energy(2049),
		_noteOnset(2049),
		_tempoDetection(2049)
	{
	}

	void init(float* audioData, int sampleRate) {
		_master.init(audioData, sampleRate);
		
		_rms.init(&_master);
		_energy.init(&_master);
		_noteOnset.init(&_master, &_energy);
		_tempoDetection.init(&_master, &_noteOnset);
	}

	void beginCalculations(int currentSample) {
		_master.beginCalculations(currentSample);
	}

	void endCalculations() {
		_master.endCalculations();
	}

	void calculateFft() {
		_master.calculateFft();
	}

	void calculateTimeConvolvedFft() {
		_master.calculateTimeConvolvedFft();
	}

	RMS _rms;
	Energy _energy;
	NoteOnset _noteOnset;
	TempoDetection _tempoDetection;

	void updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding);

	float* getFftOutput() {
		return _master._fftOutput->newest();
	}

	float* getConvolvedFftOutput() {
		return _master._timeConvolvedFftOutput.newest();
	}

	int getNumHarmonics() {
		return _master.getNumHarmonics();
	}

private:
	Master _master;
};
