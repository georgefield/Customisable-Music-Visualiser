#pragma once
#include "Tools.h"
#include "Master.h"

#include "NoteOnset.h"

class TempoDetection {
public:

	TempoDetection(int historySize) :
		_timeSinceLastBeatHistory(historySize)
	{}

	void init(Master* master, NoteOnset* noteOnset) {
		_m = master;
		_noteOnset = noteOnset;
		_sampleLastCalculated = -1;
	}

	void calculateNext();

	History<float>* getTimeSinceLastBeatHistory() {
		return &_timeSinceLastBeatHistory;
	}
private:
	Master* _m;
	NoteOnset* _noteOnset;

	int _sampleLastCalculated;

	History<float> _timeSinceLastBeatHistory;
};