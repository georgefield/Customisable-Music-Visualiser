#include "TempoDetection.h"
#include <unordered_map>

void TempoDetection::calculateNext() {

	//make sure not called twice on same frame
	if (_sampleLastCalculated == _m->_currentSample) {
		return;
	}
	_sampleLastCalculated = _m->_currentSample;

	//dependencies
	_noteOnset->calculateNext(); //use default methods

	float timeSinceLastBeat;
	timeSinceLastBeat = dixonAlgorithm();
	_timeSinceLastBeatHistory.add(timeSinceLastBeat);
}



//struct to make algorithm easier to work with
struct Cluster {
	void add(int ioi) { //inter onset interval (ioi)
		_IOI.push_back(ioi);
	}

	void mergeWith(Cluster* cluster) {
		for (int i = 0; i < cluster->_IOI.size(); i++) {
			_IOI.push_back(cluster->_IOI[i]);
		}
	}

	float interval() {
		float sum = 0;
		for (auto& it : _IOI) {
			sum += it;
		}
		sum /= _IOI.size();
		return sum;
	}

	bool merged1 = false;
	bool merged2 = false;

	std::vector<int> _IOI;
};

float TempoDetection::dixonAlgorithm() {

	std::vector<int> peaks = _noteOnset->getPeakHistory()->getAsVector();
	if (peaks.size() > 0) {
		std::cout << peaks.at(0) << std::endl; //show most recent peak
	}
	std::cout << "---" << std::endl;
	return 0.0f; //do tempo detection algorithm now

	//use symbolic representation from Dixon paper using the note onsets detected with noteOnset & peakPicking

	const float clusterWidthInSeconds = 0.05; //in samples
	const int clusterWidth = int(clusterWidthInSeconds * _m->_sampleRate);

	std::unordered_map<int, Cluster> clusters;

	//create clusters map (maps all inter onset intervals to a bin)
	for (auto& E1 : peaks) {
		for (auto& E2 : peaks) {
			if (E1 == E2) { break; }

			int ioi = fabsf(E1 - E2);
			int ci = ioi / clusterWidth;

			clusters[ci].add(ioi);
		}
	}

	//merge clusters with average interval below cluster width
	for (auto& i : clusters) {
		for (auto& j : clusters) {
			if (i.first == j.first) { break; }

			if (fabsf(i.second.interval() - j.second.interval()) < clusterWidth) {
				i.second.mergeWith(&j.second);
				j.second.merged1 = true;
			}
		}
	}

	//merge multiples
	for (auto& i : clusters) {
		if (!i.second.merged1) {
			for (auto& j : clusters) {
				if (j.second.merged2) { break; }
				if (i.first == j.first) { break; }

				if (fabsf(i.second.interval() * 2 - j.second.interval()) < clusterWidth) {
					i.second.mergeWith(&j.second);
					j.second.merged2 = true;
				}
			}
		}
	}

	return 0.0;
}
