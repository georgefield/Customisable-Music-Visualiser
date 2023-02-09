#pragma once
#include "Tools.h"
#include "Master.h"
#include "RollingAverage.h"

#include "NoteOnset.h"
#include "DixonAlgorithmStructures.h"

class TempoDetection {
public:

	TempoDetection(int historySize) :
		_tempoHistory(historySize),
		_timeSinceLastBeatHistory(historySize),
		_timeToNextBeatHistory(historySize),
		_confidenceHistory(historySize),
		_lastPeakOnset(-1),
		_initialCalculated(false),
		_tempoRollingAvg(5)
	{}

	void init(Master* master, NoteOnset* noteOnset) {
		_m = master;
		_noteOnset = noteOnset;
		_sampleLastCalculated = -1;
	}

	void calculateNext();

	bool hasData() { //may not have had enough time to have any data yet
		return _initialCalculated;
	}

	History<float>* getTimeSinceLastBeatHistory() {
		return &_timeSinceLastBeatHistory;
	}
	History<float>* getTimeToNextBeatHistory() {
		return &_timeToNextBeatHistory;
	}
	History<float>* getTempoHistory() {
		return &_tempoHistory;
	}
	History<float>* getConfidenceInTempoHistory() {
		return &_confidenceHistory;
	}
private:
	RollingAverage _tempoRollingAvg;

	Master* _m;
	NoteOnset* _noteOnset;

	bool _initialCalculated;
	int _sampleLastCalculated;

	History<float> _tempoHistory; //in bpm
	History<float> _timeSinceLastBeatHistory; //in seconds
	History<float> _timeToNextBeatHistory; //in seconds
	History<float> _confidenceHistory; //from 0 to 1

	float _clusterRadiusInSeconds;
	ClusterSet* _clusters;
	AgentSet* _agents;

	std::vector<Peak> _peaks;
	int _lastPeakOnset;
	void initialDixonAlg();
	void continuousDixonAlg();
	void computeClusters(ClusterSet* clusters, std::vector<Peak>& peaks);
	void computeAgents(AgentSet* agents, std::vector<Peak>& peaks);
};