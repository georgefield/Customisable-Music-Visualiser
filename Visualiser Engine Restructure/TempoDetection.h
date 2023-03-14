#pragma once
#include "Tools.h"
#include "Master.h"
#include "RollingAverage.h"

#include "NoteOnset.h"
#include "DixonAlgorithmStructures.h"
#include "imgui.h"

class TempoDetection {
public:

	TempoDetection() :
		_tempoRollingAvg(5),
		_confidenceRollingAvg(8)
	{}

	~TempoDetection() {
		removeUpdaters();
	}

	void init(Master* master, NoteOnset* noteOnset) {
		_m = master;
		_noteOnset = noteOnset;

		_sampleLastCalculated = -1;
		_lastPeakOnset = -1;
		_howManyPeaks = 0;

		_tempo = 100;
		_tempoConfidence = 0.0f;
		_timeSinceLastBeat = 0.0f;
		_timeToNextBeat = 0.0f;

		_initialCalculated = false;

		//set up clusters and agents--
		int clusterRadius = int(DixonAlgVars::CLUSTER_RADIUS_SECONDS * _m->_sampleRate); //need radius in samples
		_clusters = new ClusterSet(clusterRadius);

		_agents = new AgentSet(_m->_sampleRate);
		//--

		initUpdaters();
	}

	void reInit() {
		_sampleLastCalculated = -1;
		_lastPeakOnset = -1;
		_howManyPeaks = 0;

		_tempo = 100;
		_tempoConfidence = 0.0f;
		_timeSinceLastBeat = 0.0f;
		_timeToNextBeat = 0.0f;

		_initialCalculated = false;

		//set up structures again--
		delete _clusters;
		delete _agents;

		int clusterRadius = int(DixonAlgVars::CLUSTER_RADIUS_SECONDS * _m->_sampleRate); //need radius in samples
		_clusters = new ClusterSet(clusterRadius);

		_agents = new AgentSet(_m->_sampleRate);
		//--

		//clear data--
		_peaks.clear();
		_tempoRollingAvg.clear();
		_confidenceRollingAvg.clear();
		//--
	}

	void calculateNext();

	bool hasData() { //may not have had enough time to have any data yet
		return _initialCalculated;
	}

	float getTimeSinceLastBeat() {
		return _timeSinceLastBeat;
	}
	float getTimeToNextBeat() {
		return _timeToNextBeat;
	}
	float getTempo() {
		return _tempo;
	}
	float getConfidenceInTempo() {
		return _tempoConfidence;
	}

	void getDebugInfo(std::vector<std::string>& debugInfo) {

		if (!_initialCalculated) {
			debugInfo.push_back("Not enough peak data to calculate tempo");
			return;
		}

		_agents->debug3(debugInfo, _m->_sampleRate);
	}

private:

	Master* _m;
	NoteOnset* _noteOnset;

	bool _initialCalculated;
	int _sampleLastCalculated;

	//calculated values--
	RollingAverage _tempoRollingAvg;
	RollingAverage _confidenceRollingAvg;

	float _tempo;
	float _timeSinceLastBeat;
	float _timeToNextBeat;
	float _tempoConfidence;
	//--

	ClusterSet* _clusters;
	AgentSet* _agents;

	std::vector<Peak> _peaks;
	int _lastPeakOnset;
	int _howManyPeaks;

	void initialDixonAlg();
	void continuousDixonAlg();
	void computeClusters(ClusterSet* clusters, std::vector<Peak>& peaks);
	void computeAgents(AgentSet* agents, std::vector<Peak>& peaks);

	void initUpdaters();
	void removeUpdaters();
};