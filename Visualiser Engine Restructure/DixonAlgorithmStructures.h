#pragma once
#include <Vengine/MyErrors.h>
#include <algorithm>
#include <list>
#include <vector>
#include <iostream>

#include "NoteOnset.h"
#include "Peak.h"
#include "Tools.h"

struct DixonAlgVars {
	static float CLUSTER_RADIUS_SECONDS;
	static int MAX_PEAKS_STORED;
	static float SCORE_FACTOR_PER_NEW_BEAT;
	static int MAX_AGENTS_STORED;
	static float ACCOUNT_FOR_CLUSTER_SCORE; //between 0 and 1, 1 is fully and 0 is none
	static float ERROR_CORRECTION_AMOUNT;
	static int NUM_PEAKS_NEEDED_BEFORE_START;
	static int MAX_AGENT_HISTORY_LENGTH;
	static float SECONDS_UNTIL_TIMEOUT;
	static float MAX_TIME_PEAKS_SCORED;
};


//returns false if interval too big too small (implies tempo out of range), used in structures and tempoDetection.cpp
struct DixonAlgFunc {
	static bool intervalImpliesValidTempo(int interval, int sampleRate) {
		if (interval >= (60.0f / SPvars::UI::MAX_TEMPO) * sampleRate &&
			interval <= (60.0f / SPvars::UI::MIN_TEMPO) * sampleRate) {
			return true;
		}
		return false;
	}
};


//*** Cluster struct and container

struct Cluster {
	Cluster(int firstEntry) :
		_sum(0),
		_score(0)
	{
		add(firstEntry);
	}

	//push to array then update avg interval
	void add(int ioi) {
		_IOIs.push_back(ioi);
		_sum += ioi;
		_avgInterval = _sum / _IOIs.size();
	}

	//adding multiple intervals at once
	void add(std::vector<int>* iois) {
		for (auto it : *iois) {
			_IOIs.push_back(it);
			_sum += it;
		}
		_avgInterval = _sum / _IOIs.size();
	}

	//merge clusters
	void mergeWith(Cluster* cluster) {
		add(&cluster->_IOIs);
	}

	float _avgInterval;
	float _score;

	std::vector<int> _IOIs;
private:
	int _sum;
};

struct ClusterSet {
	ClusterSet(int clusterRadius) :
		_clusterRadius(clusterRadius)
	{}

	int _clusterRadius;
	std::list<Cluster> set;

	void debug() {
		for (auto it : set) {
			std::cout << it._avgInterval << std::endl;
		}
		std::cout << std::endl;
	}

	void debug2() {
		for (auto it : set) {
			std::cout << it._avgInterval << " < interval, score > " << it._score << std::endl;
		}
	}

	void add(int ioi) {
		//keeps clusters sorted by interval when adding. Improves speed for merging.
		//handle first addition
		if (set.size() == 0) {
			set.emplace_back(ioi);
			return;
		}

		//get nearest cluster
		std::list<Cluster>::iterator nearest;
		int distanceToNearest;
		bool isNearestAbove;
		getClusterWithClosestInterval(ioi, nearest, isNearestAbove, distanceToNearest);

		//close enough to add to existing cluster
		if (distanceToNearest < _clusterRadius) {
			(*nearest).add(ioi);
			return;
		}

		//smaller than all
		if (nearest == set.begin() && isNearestAbove) {
			set.emplace_front(ioi);
			return;
		}

		//bigger than all
		if (nearest == std::prev(set.end()) && !isNearestAbove) {
			set.emplace_back(ioi);
			return;
		}


		//in between
		auto addPosition = nearest;
		if (isNearestAbove) {
			addPosition--; //decrement if nearest above as inserts after cluster pointed to
		}
		set.emplace(std::next(addPosition), ioi);
	}

	Cluster* bestCluster() {
		Cluster* best = nullptr;
		for (auto& it : set) {
			if (best == nullptr) {
				best = &it;
			}
			else if (best != nullptr && it._score > best->_score) {
				best = &it;
			}
		}
		return best;
	}

	void getClusterWithClosestInterval(int ioi, std::list<Cluster>::iterator& nearest, bool& isNearestAbove, int& distanceToNearest) {

		//get above and below clusters
		std::list<Cluster>::iterator above = std::lower_bound(set.begin(), set.end(), Cluster(ioi), [](const Cluster& a, const Cluster& b) { return a._avgInterval < b._avgInterval; });
		std::list<Cluster>::iterator below;
		if (above == set.begin())
			below = set.begin();
		else
			below = std::prev(above);

		//handle smaller than all and larger than all
		if (above == set.end()) { //add to end
			nearest = below;
			isNearestAbove = false;
			distanceToNearest = ioi - (*below)._avgInterval;
		}
		else if (above == set.begin()) { //add to beginning
			nearest = above;
			isNearestAbove = true;
			distanceToNearest = (*above)._avgInterval - ioi;
		}
		else {
			//inbetween
			int distanceToAbove = (*above)._avgInterval - ioi;
			int distanceToBelow = ioi - (*below)._avgInterval;
			if (distanceToBelow < distanceToAbove) {
				nearest = below;
				isNearestAbove = false;
				distanceToNearest = distanceToBelow;
			}
			else {
				nearest = above;
				isNearestAbove = true;
				distanceToNearest = distanceToAbove;
			}
		}
	}

	void mergeNearbyClusters() {
		//stop error caused by std::next if too small
		if (set.size() <= 1) {
			Vengine::warning("Num clusters <= 1");
			return;
		}

		//linear time because sorted
		auto it = set.begin();
		while (it != set.end() && std::next(it) != set.end()) { //it might become end as erase the one above after merging so check first
			auto itAbove = std::next(it);
			//std::cout << (*itAbove)._avgInterval << " - " << (*it)._avgInterval <<  " < "<< _clusterRadius << "?" << std::endl;
			//no problems with delete elements of list being looped through as using iterator
			if ((*itAbove)._avgInterval - (*it)._avgInterval < _clusterRadius) { //if intervals closer than cluster radius
				(*it).mergeWith(&(*itAbove)); //then merge
				set.erase(itAbove);
				//std::cout << "MERGED" << std::endl;
			}
			else {
				//std::cout << "NOT MERGED" << std::endl;
			}
			it++; //increment iterator
		}
	}

	void reset() {
		set.clear();
	}
};


//*** Agent struct and container ***

struct Agent {
	Agent(int interval) :
		_beatInterval(interval),

		_accountingForIntervalScore(0.0f),

		_score(0),
		_prediction(0)
	{
		_peakHistory = new History<float>(DixonAlgVars::MAX_AGENT_HISTORY_LENGTH);
		id = rand();
	}

	~Agent() {
		delete _peakHistory;
	}

	void add(Peak peak, int timingError, float relativeError) {

		_beatInterval += timingError * DixonAlgVars::ERROR_CORRECTION_AMOUNT;
		_prediction = peak.onset + _beatInterval;

		peak.salience *= (1.0f - relativeError);

		_score += peak.salience;

		_peakHistory->add(peak.salience, peak.onset);
	}

	void updatePeaks(int sampleRate, int currentSample) {
		while (_peakHistory->entries() > 0 && currentSample - _peakHistory->oldestSample() > sampleRate * DixonAlgVars::MAX_TIME_PEAKS_SCORED) {
			_score -= _peakHistory->oldest();
			_peakHistory->removeOldest();
		}
	}

	int id; //helps with debug
	int _beatInterval;
	int _prediction;
	float _score;
	float _accountingForIntervalScore;
	History<float>* _peakHistory;
};


struct AgentSet {

	AgentSet(int sampleRate) :
		_sampleRate(sampleRate),
		_highestScoringAgent(nullptr),
		_confidenceInBestAgent(0.0f),
		_sortedBy(UNSORTED)
	{
		_tempoMinDifference = 0.01 * _sampleRate; //20ms
		_phaseMinDifference = 0.02 * _sampleRate; //40ms
	}

	enum SortedBy {
		UNSORTED,
		PREDICTION,
		COMPOUND_SCORE
	};

	Agent* _highestScoringAgent;
	float _confidenceInBestAgent;
	std::list<Agent*> set;
	SortedBy _sortedBy;

	/*
	void debug() {
		for (auto& it : set) {
			std::cout << it._score << std::endl;
		}
		std::cout << std::endl;
	}

	void debug2() {
		for (auto it = set.begin(); it != set.end(); it++) {
			std::cout << "bi, prd, sc" << it->_beatInterval << ", " << it->_prediction << ", " << it->_accountingForIntervalScore << std::endl;
		}
	}
	*/
	void debug3(std::vector<std::string>& debugInfo, int sampleRate) {
		sortAgentsByScoresAccountingForIntervalScores();

		int count = 1;
		for (auto it = set.rbegin(); it != set.rend(); it++) {
			std::string info = "Rank " + std::to_string(count) + ": tempo = " + std::to_string(float(60 * sampleRate) / float((*it)->_beatInterval)) + ", score = " + std::to_string((*it)->_accountingForIntervalScore) + ", id = " + std::to_string((*it)->id);
			debugInfo.push_back(info);
			count++;
		}
	}

	void prepareForCompute() {
		if (_sortedBy != PREDICTION) {
			sortAgentsByPrediction();
		}
	}

	void removeOldPeaksFromAgents(int currentSample) {
		for (auto& it : set) {
			it->updatePeaks(_sampleRate, currentSample);
		}
	}

	void add(int beatInterval, Peak firstEventToAdd) {

		set.push_back(new Agent(beatInterval));
		set.back()->add(firstEventToAdd, 0, 0);

		//std::cout << "added " << set.back()->id << std::endl;

		_sortedBy = UNSORTED;
	}

	void storeUnmodifiedVersionOfAgent(Agent* agent) {

		//copy to temporary array
		pushToSetOnEnd.push_back(new Agent(agent->_beatInterval));
		for (int i = 0; i < agent->_peakHistory->entries(); i++) {
			pushToSetOnEnd.back()->_peakHistory->add(agent->_peakHistory->get(i));
		}
		pushToSetOnEnd.back()->_score = agent->_score;

		//std::cout << agent->id << " stored unmod. as " << pushToSetOnEnd.back()->id <<  std::endl;
	}

	void addBackTheUnmodifiedAgents() {
		for (auto& it : pushToSetOnEnd) {
			//std::cout << it->id << " added back" << std::endl;
			set.push_back(it);
		}
		_sortedBy = UNSORTED;

		pushToSetOnEnd.clear();
	}

	void removeDuplicateAgents() {

		if (_sortedBy != PREDICTION) {
			sortAgentsByPrediction();
		}


		//prevent errors caused by set being too small
		if (set.size() <= 1) {
			Vengine::warning("1 or 0 agents in set");
			return;
		}


		for (auto A1 = set.begin(); A1 != set.end(); A1++) {
			auto A2 = std::next(A1);

			//we know A2.prediction > A1.prediction, and if A2.prediction > A1.prediction, no point incrementing A2 as difference will be more (because agents sorted by prediction)
			while (A2 != set.end() && ((*A2)->_prediction - (*A1)->_prediction) < _phaseMinDifference) {
				//for every pair of agents

				//erase every agent with phase difference & tempo difference both too similar
				//std::cout << (*A1)->id << "<A1 A2>" << (*A2)->id << std::endl;
				if (fabsf((*A1)->_beatInterval - (*A2)->_beatInterval) < _tempoMinDifference) {

					//std::cout << "agent removed " << (*A1)._beatInterval << " = " << (*A2)._beatInterval << ", " << (*A1)._prediction << "=" << (*A2)._prediction << "," << set.size() << " agents left" << std::endl;

					//keep agent with highest score
					if ((*A1)->_score < (*A2)->_score) {
						//std::cout << "swapped then ";
						auto tmp = (*A1);
						(*A1) = (*A2);
						(*A2) = tmp;
					}
					//std::cout << (*A2)->id << " removed duplicate. keeping " << (*A1)->id << std::endl;

					A2 = erase(A2);
				}
				else {
					A2++;
				}
			}
		}
	}

	void removeTimedOutAgents(Peak lastPeak) {
		for (auto A = set.begin(); A != set.end(); ) {
			if (lastPeak.onset - (*A)->_peakHistory->newestSample() > DixonAlgVars::SECONDS_UNTIL_TIMEOUT * _sampleRate) {
				A = erase(A);
			}
			else {
				A++;
			}
		}
	}

	void removeAgentsWithBadTempo() {
		for (auto A = set.begin(); A != set.end(); ) {
			if(!DixonAlgFunc::intervalImpliesValidTempo((*A)->_beatInterval, _sampleRate) ){
				A = erase(A);
			}
			else {
				A++;
			}
		}
	}

	void calculateScoresAccountingForClusterIntervalScores(ClusterSet* clusters) {
		if (set.size() == 0) {
			Vengine::warning("No agents in set");
			return;
		}

		float bestClusterScore = clusters->bestCluster()->_score;

		std::list<Cluster>::iterator nearest;
		bool isNearestAbove;
		int distanceToNearest;
		for (auto it = set.begin(); it != set.end(); it++) {
			clusters->getClusterWithClosestInterval((*it)->_beatInterval, nearest, isNearestAbove, distanceToNearest);
			float relativeErrorFromClosest;
			if (isNearestAbove) {
				relativeErrorFromClosest = 1.0f - (float(distanceToNearest) / float((*it)->_beatInterval));
			}
			else {
				relativeErrorFromClosest = 1.0f - (float(distanceToNearest) / float((*nearest)._avgInterval));
			}
			float clusterScore = (relativeErrorFromClosest * (*nearest)._score);
			float score = (*it)->_score;
			(*it)->_accountingForIntervalScore =
				(1.0f - DixonAlgVars::ACCOUNT_FOR_CLUSTER_SCORE) * bestClusterScore * score +
				DixonAlgVars::ACCOUNT_FOR_CLUSTER_SCORE * score * clusterScore;

			//set highest scoring agent variable
			if (_highestScoringAgent == nullptr || (*it)->_accountingForIntervalScore > _highestScoringAgent->_accountingForIntervalScore) {
				_highestScoringAgent = (*it);
			}
		}
	}

	void sortAgentsByScoresAccountingForIntervalScores() {
		set.sort([](const Agent* a, const Agent* b) { return a->_accountingForIntervalScore < b->_accountingForIntervalScore; });
		_sortedBy = COMPOUND_SCORE;
	}

	void sortAgentsByPrediction() {
		set.sort([](const Agent* a, const Agent* b) { return a->_prediction < b->_prediction; });
		_sortedBy = PREDICTION;
	}

	void insertionSortAgentsByPrediction() { //error correction might change a few values but list will still be almost sorted so use insertion sort
		for (auto i = set.begin(); i != set.end(); ++i) {
			auto j = i;
			while (j != set.begin() && (*std::prev(j))->_prediction > (*j)->_prediction) {
				std::swap(*j, *std::prev(j));
				--j;
			}
		}
		_sortedBy = PREDICTION;
	}

	void calculateConfidenceInBestAgent(int clusterRadius) { //increase cluster radius bruh bruh bruh
		if (set.size() <= 1) {
			_confidenceInBestAgent = set.size();
			return;
		}
		auto it = set.end();
		it--;
		int bestInterval = _highestScoringAgent->_beatInterval;
		int skips = 0;
		do {
			
			if (it == set.begin()) {
				_confidenceInBestAgent = 1.0f;
				return;
			}
			it--;
			if (fabsf((*it)->_beatInterval - bestInterval) < (2 * clusterRadius)) {
				skips++;
			}
		} while (fabsf((*it)->_beatInterval - bestInterval) < (2 * clusterRadius)); //2* garuntees that agents accounting for same cluster inteval score skipped to first thats not

		float secondBestScore = (*it)->_accountingForIntervalScore; //first agent with interval not near best agent interval

		_confidenceInBestAgent = (1.0f - (secondBestScore / _highestScoringAgent->_accountingForIntervalScore)) * std::min(_highestScoringAgent->_accountingForIntervalScore / 2000.0f, 1.0f); //vs 2nd best thats very different & also making sure good minimum score
		//_confidenceInBestAgent *= 1.5f; //even in best case not better than 2/3 because half tempo always
	}

	void shrinkSetToSize(int size) {
		if (_sortedBy != COMPOUND_SCORE) {
			sortAgentsByScoresAccountingForIntervalScores();
		}

		if (set.size() <= size) {
			return;
		}

		int howManyToErase = (set.size() - size);
		for (int i = 0; i < howManyToErase; i++) {
			erase(set.begin()); //erase lowest scores
		}
	}

	void devalueScores(float factor) {
		for (auto it = set.begin(); it != set.end(); it++) {
			(*it)->_score *= factor;
		}
	}
private:
	//duplicate thresholds
	int _tempoMinDifference;
	int _phaseMinDifference;

	int _sampleRate;

	std::vector<Agent*> pushToSetOnEnd;

	std::list<Agent*>::iterator erase(std::list<Agent*>::iterator A) {
		//std::cout << "Erasing id: " << (*A)->id << std::endl;
		delete (*A);
		return set.erase(A);
	}
};