#include "TempoDetection.h"

#include <unordered_map>
#include <algorithm>

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

//*** dixon algorithm ***

//relationship function
const int TEST_RATIOS_UP_TO = 8;
int f(int d) {
	if (d > 8)
		return 0;
	if (d > 4)
		return 1;
	if (d > 0)
		return 6 - d;
	Vengine::warning("negative d value for relationship function");
	return 0;
}


//struct to make algorithm easier to work with
struct Cluster {
	Cluster(int firstEntry) :
		_sum(0),
		score(0)
	{
		add(firstEntry);
	}

	void add(int ioi) { //inter onset interval (ioi)
		IOIs.push_back(ioi);
		_sum += ioi;
		interval = _sum / IOIs.size();
	}

	void add(std::vector<int>* iois) { //adding multiple intervals at once
		for (auto it : *iois) {
			IOIs.push_back(it);
			_sum += it;
		}
		interval = _sum / IOIs.size();
	}

	void mergeWith(Cluster* cluster) {
		add(&cluster->IOIs);
	}

	float interval;
	float score;

	std::vector<int> IOIs;
private:
	int _sum;
};

bool compareClusterByInterval(const Cluster& a, const Cluster& b) {
	return a.interval < b.interval;
}

struct ClusterSet {
	ClusterSet(int clusterRadius) :
		_clusterRadius(clusterRadius)
	{}

	std::list<Cluster> set;

	void debug() {
		for (auto it : set) {
			std::cout << it.interval << std::endl;
		}
		std::cout << std::endl;
	}

	void debug2() {
		for (auto it : set) {
			std::cout << it.interval << " < interval, score > " << it.score << std::endl;
		}
	}

	void add(int ioi) {
		if (set.size() == 0) {
			std::cout << "first added at beginning: " << ioi << std::endl;
			set.emplace(set.begin(), ioi);
			return;
		}

		auto it = std::lower_bound(set.begin(), set.end(), ioi, compareClusterByInterval);

		//special cases for smallest so far or biggest so far
		if (it == set.begin()) { //smallest so far
			int closestAbove = (*it).interval;
			int distanceAbove = closestAbove - ioi;

			if (distanceAbove > _clusterRadius) {
				std::cout << "added " << ioi << " at front, in front of " << (*it).interval << std::endl;
				set.emplace_front(ioi); //new cluster at beginning
				return;
			}
			std::cout << "adding interval " << ioi << " to start cluster " << (*it).interval << std::endl;
			(*it).add(ioi);
			return;
		}

		if (it == set.end()) {
			it--; //it now points to closest below
			int closestBelow = (*it).interval;
			int distanceBelow = ioi - closestBelow;

			if (distanceBelow > _clusterRadius) {
				std::cout << "added " << ioi << " at back, behind " << (*it).interval << std::endl;
				set.emplace_back(ioi); //new cluster at end
				return;
			}
			std::cout << "adding interval " << ioi << " to end cluster " << (*it).interval << std::endl;
			(*it).add(ioi);
			return;
		}

		//for ioi is inbetween cluster avg iois
		auto itBelow = std::prev(it);

		int closestAbove = (*it).interval;
		int closestBelow = (*itBelow).interval;

		int distanceAbove = closestAbove - ioi;
		int distanceBelow = ioi - closestBelow;

		if (distanceAbove > _clusterRadius && distanceBelow > _clusterRadius) { //no cluster in range
			std::cout << "added " << ioi << " between " << (*itBelow).interval << " and " << (*it).interval << std::endl;

			set.emplace(it, ioi); //new cluster inbetween
			return;
		}

		if (distanceAbove < distanceBelow) {
			std::cout << "adding interval " << ioi << " to cluster " << (*it).interval << std::endl;
			(*it).add(ioi); //add ioi to cluster above
			return;
		}
		std::cout << "adding interval " << ioi << " to cluster " << (*itBelow).interval << std::endl;
		(*itBelow).add(ioi); //add ioi to cluster below
		return;
	}

	void mergeNearbyClusters() {
		if (set.size() <= 1) {
			Vengine::warning("Num clusters <= 1");
			return;
		}

		auto it = set.begin();
		while (it != set.end() && std::next(it) != set.end()) { //it might become end as erase the one above after merging so check first
			auto itAbove = std::next(it);
			//no problems with delete elements of list being looped through as using iterator
			if ((*itAbove).interval - (*it).interval < _clusterRadius) { //if intervals closer than cluster radius
				(*it).mergeWith(&(*itAbove)); //then merge
				set.erase(itAbove);
			}
			it++; //increment iterator
		}
	}
private:
	int _clusterRadius;
};

struct Agent {
	Agent(int beatInterval, Peak firstEventToAdd) :
		_beatInterval(beatInterval),
		_prediction(firstEventToAdd.onset + beatInterval)
	{
		_history.push_back(firstEventToAdd);
		_score = firstEventToAdd.salience;
	}

	int _beatInterval;
	int _prediction;
	std::vector<Peak> _history;
	float _score;

};

struct AgentSet {

	AgentSet(int sampleRate) :
		_sampleRate(sampleRate)
	{
		_tempoMinDifference = 0.01 * _sampleRate; //10ms
		_phaseMinDifference = 0.02 * _sampleRate; //20ms
	}

	void add(int beatInterval, Peak firstEventToAdd) {
		set.emplace_back(beatInterval, firstEventToAdd);
	}

	void copyToNewAgent(std::list<Agent>::iterator agent) {
		pushToSetOnEnd.emplace_back((*agent)._beatInterval, (*agent)._history.front());

		for (int i = 1; i < (*agent)._history.size(); i++) { //first history beat added already
			pushToSetOnEnd.back()._history.push_back((*agent)._history[i]);
		}

		pushToSetOnEnd.back()._score = (*agent)._score;
	}

	void addNewAgents() {
		for (auto it : pushToSetOnEnd) {
			set.emplace_back(it);
		}
		pushToSetOnEnd.clear();
	}

	void removeDuplicateAgents() {
		for (auto A1 = set.begin(); std::next(A1) != set.end(); ++A1) {
			for (auto A2 = std::next(A1); A2 != set.end(); ++A2) {
				//for every pair of agents

				//if too similar beat interval and prediction times then remove the one with the lowest score
				if (fabsf((*A1)._beatInterval - (*A2)._beatInterval) < _tempoMinDifference &&
					fabsf((*A1)._prediction - (*A2)._prediction) < _phaseMinDifference) {

					if ((*A1)._score > (*A2)._score) { //always erase A2 then break
						set.erase(A2);
						break;
					}
					else { //A2 score better, replace A1 with A2 then erase A2
						(*A1) = (*A2);
						set.erase(A2);
						break;
					}
				}
			}
		}
	}

	Agent* getHighestScoringAgent() {
		if (set.size() == 0) {
			Vengine::warning("No agents");
			return nullptr;
		}

		Agent* bestAgent = &(*set.begin());
		for (auto A = set.begin(); A != set.end(); ++A) {
			if ((*A)._score > bestAgent->_score) {
				bestAgent = &(*A);
			}
		}

		return bestAgent;
	}

	std::list<Agent> set;
private:
	//duplicate thresholds
	int _tempoMinDifference;
	int _phaseMinDifference;

	int _sampleRate;

	std::vector<Agent> pushToSetOnEnd;
};

float TempoDetection::dixonAlgorithm() {

	std::vector<Peak> peaks;// = _noteOnset->getPeakHistory()->getAsVector();

	const int hardCodedPeaks[] = {
		5345,
		7000,
		85000,
		100000,
		120000,
		140000,
		160000,
		30000,
		50000,
		55000,
		141000,
		200000,
		219850,
		240500
	};
	const int hardCodedPeaksSize = sizeof(hardCodedPeaks) / sizeof(int);
	peaks.reserve(hardCodedPeaksSize);
	for (int i = 0; i < hardCodedPeaksSize; i++) {
		peaks.push_back({ hardCodedPeaks[i], 1.0f });
	}
	//use symbolic representation from Dixon paper using the note onsets detected with noteOnset & peakPicking

	//*** tempo induction ***

	const float clusterRadiusInSeconds = 0.025; //allowed error from average ioi of cluster to be added
	const int clusterRadius = int(clusterRadiusInSeconds * _m->_sampleRate);

	ClusterSet clusters(clusterRadius);

	//compare intervals off all peaks
	for (auto& E1 : peaks) {
		for (auto& E2 : peaks) {
			if (E1.onset == E2.onset) { break; }

			int ioi = fabsf(E1.onset - E2.onset);
			clusters.add(ioi); //handles which cluster its added to
		}
	}


	//merge clusters with average interval below cluster width
	clusters.mergeNearbyClusters();

	//score clusters
	for (auto rit = clusters.set.rbegin(); rit != clusters.set.rend(); ++rit) {
		
		std::cout << (*rit).interval << ": ";

		//set base score depends on how many intervals in cluster
		(*rit).score += f(1) * (*rit).IOIs.size();

		for (auto it = clusters.set.begin(); std::next(it) != rit.base(); ++it) {

			std::cout << " " << (*it).interval;

			//test if itBelow interval factor of it
			for (int k = 2; k <= TEST_RATIOS_UP_TO; k++) {
				//= it.interval - (k * itBelow.interval)
				float difference = (*rit).interval - (k * ((*it).interval));

				//if |it.interval - k*itBelow.interval| < clusterRadius
				if (fabsf(difference) <= clusterRadius) {
					std::cout << f(k) << " " << (*rit).IOIs.size() << std::endl;
 					(*it).score += f(k) * (*rit).IOIs.size();
				}
			}
		}

		std::cout << std::endl;
	} 

	clusters.debug2();
	//ready for next part (AYO)

	//*** phase induction ***
	AgentSet agents(_m->_sampleRate);

	for (auto it = clusters.set.begin(); it != clusters.set.end(); ++it) {
		for (auto& peak : peaks) {
			agents.add((*it).interval, peak);
		}
	}

	float tolPrePercentage = 0.2; //20%
	float tolPostPercentage = 0.4; //40%

	int tolInner = 0.04 * _m->_sampleRate; // 40 ms

	int timeOut = 2 * _m->_sampleRate; //how long after last peak that aligns with beat hypothesis before deleteing that agent

	float correctionFactor = 5; //how fast tempo changes to incorporate new beat information

	for (auto& peak : peaks) {
		std::cout << "peak: " << peak.onset << std::endl;
		for (auto A = agents.set.begin(); A != agents.set.end(); ) {
			int tolPre = tolPrePercentage * (*A)._beatInterval;
			int tolPost = tolPostPercentage * (*A)._beatInterval;
			std::cout << "tol pre " << tolPre << " post " << tolPost << std::endl;

			if (peak.onset - (*A)._history.back().onset > timeOut) {
				auto itBefore = std::prev(A);
				agents.set.erase(A);
				A = itBefore; //go back one (know it exists)
				++A; //increment
			}
			else {
				std::cout << "agent pred " << (*A)._prediction << " peak " << peak.onset;
				while ((*A)._prediction + tolPost < peak.onset) {
					(*A)._prediction += (*A)._beatInterval;
					(*A)._score -= 0.3; //negative equivalent of hitting a small beat
				}
				if ((*A)._prediction - tolPre <= peak.onset && peak.onset <= (*A)._prediction + tolPost) {
					if (fabsf((*A)._prediction - peak.onset) > tolInner) { //outside in tolerance but inside outer tolerance
						agents.copyToNewAgent(A); //create new agent that does not count this peak as beat time to protect against wrong decision
						std::cout << " not inner, ";
					}
					std::cout << " in outer";

					//count this peak as a beat time
					int error = peak.onset - (*A)._prediction;
					float relativeError = error / (*A)._beatInterval;

					(*A)._beatInterval += error / correctionFactor;
					(*A)._prediction = peak.onset + (*A)._beatInterval;
					(*A)._history.push_back(peak);
					(*A)._score += (1 - (relativeError * 0.5)) * peak.salience;
				}
				else {
					std::cout << " ignored";
				}
				++A; //increment
			}
			std::cout << std::endl << "---" << std::endl;

		}
		agents.addNewAgents();
		agents.removeDuplicateAgents();
	}
	Agent* bestAgent = agents.getHighestScoringAgent();
	std::cout << "bi, prd, sc" << bestAgent->_beatInterval << ", " << bestAgent->_prediction << ", " << bestAgent->_score << std::endl;

	system("PAUSE");

	return 0.0;
}
