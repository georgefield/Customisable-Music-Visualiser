#pragma once
#include "Master.h"
#include "FFTs.h"
#include "NoteOnset.h"
#include "Energy.hpp"
#include "RMS.hpp"
#include "TempoDetection.h"

#include <GL/glew.h>

class SignalProcessing {
public:
	SignalProcessing() :
		_master(),
		_rms(2049),
		_energy(2049),
		_noteOnset(2049),
		_tempoDetection(2049),
		_FFTs(20, 4096)
	{
	}

	void init(float* audioData, int sampleRate) {
		_master.init(audioData, sampleRate);
		
		_FFTs.init(&_master);
		_rms.init(&_master);
		_energy.init(&_master);
		_noteOnset.init(&_master, &_energy, &_FFTs);
		_tempoDetection.init(&_master, &_noteOnset);
	}

	void beginCalculations(int currentSample) {
		_master.beginCalculations(currentSample);
	}

	void endCalculations() {
		_master.endCalculations();
	}

	FFTs _FFTs;
	RMS _rms;
	Energy _energy;
	NoteOnset _noteOnset;
	TempoDetection _tempoDetection;

	void updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding);

	int getNumHarmonics() {
		return _FFTs._numHarmonics;
	}

private:
	Master _master;
};
