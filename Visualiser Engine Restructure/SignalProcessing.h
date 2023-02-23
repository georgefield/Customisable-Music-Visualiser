#pragma once
#include "Master.h"
#include "NoteOnset.h"
#include "Energy.hpp"
#include "RMS.hpp"
#include "TempoDetection.h"
#include "FourierTransform.h"
#include "MFCCs.h"
#include "SelfSimilarityMatrix.h"

#include <GL/glew.h>
#include <unordered_map>

class SignalProcessing {
public:
	SignalProcessing() :
		_master(),
		_rms(2049),
		_energy(2049),
		_noteOnset(2049),
		_tempoDetection(2049),
		_selfSimilarityMatrix(500)
	{
	}

	void init(float* audioData, int sampleRate) {
		_master.init(audioData, sampleRate);
		
		_rms.init(&_master);
		_energy.init(&_master);
		_noteOnset.init(&_master, &_energy);
		_tempoDetection.init(&_master, &_noteOnset);
		_mfccs.init(&_master, 25, 0, 20000);
	}


	void beginCalculations(int currentSample) {
		if (currentSample < 0) {
			Vengine::warning("currentSample parameter passed to signal processor class was negative");
		}
		_master.beginCalculations(currentSample);
		_master.calculateFourierTransform(); //always calculate fourier
	}

	void endCalculations() {
		_master.endCalculations();
	}

	Master* getMaster() {
		return &_master;
	}

	RMS _rms;
	Energy _energy;
	NoteOnset _noteOnset;
	TempoDetection _tempoDetection;
	MFCCs _mfccs;
	SelfSimilarityMatrix _selfSimilarityMatrix;

	void updateSSBOwithHistory(History<float>* history, GLuint id, GLint binding);
	void updateSSBOwithVector(std::vector<float> vector, GLuint id, GLint binding); //slow shouldnt use

private:
	Master _master;

	std::unordered_map<int, FourierTransform*> _fourierTransforms;
};
