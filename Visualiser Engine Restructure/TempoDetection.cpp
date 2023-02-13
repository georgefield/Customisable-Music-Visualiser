#include "TempoDetection.h"

#include <unordered_map>
#include <algorithm>

//use these to check alg working, should be score 12.832, accounted score 5746.29, interval 22151, when using peaks index 10 to 49
static const Peak debugPeaks[] = {
	{173531, 0.299334},
	{195338, 0.34997},
	{217155, 0.279703},
	{239172, 0.279988},
	{250790, 0.252536},
	{261346, 0.177285},
	{282988, 0.380858},
	{305269, 0.359866},
	{326898, 0.388706},
	{349080, 0.313638},
	{371298, 0.358589},
	{392958, 0.395626},
	{414888, 0.281313},
	{426662, 0.186369},
	{437540, 0.304242},
	{459024, 0.398928},
	{480863, 0.305505},
	{502575, 0.419643},
	{524868, 0.353359},
	{546674, 0.455131},
	{568776, 0.309439},
	{590375, 0.352784},
	{612734, 0.346797},
	{634445, 0.352873},
	{656628, 0.266307},
	{678437, 0.397714},
	{700579, 0.406734},
	{722748, 0.347926},
	{744445, 0.343726},
	{766162, 0.370122},
	{778061, 0.339499},
	{789066, 0.30836},
	{810348, 0.294004},
	{832216, 0.325078},
	{854668, 0.403057},
	{876153, 0.361749},
	{898380, 0.416377},
	{920731, 0.342708},
	{942031, 0.381639},
	{964156, 0.296897},
	{986092, 0.391958},
	{1008048, 0.33328},
	{1029966, 0.292398},
	{1052052, 0.2897},
	{1074607, 0.479422},
	{1096247, 0.355332},
	{1118082, 0.343106},
	{1129856, 0.268347},
	{1140983, 0.372489},
	{1152125, 0.23371},
	{1162350, 0.429268},
	{1173809, 0.256634},
	{1184324, 0.343433}
};


void TempoDetection::calculateNext() {

	//make sure not called twice on same frame
	if (_sampleLastCalculated == _m->_currentSample) {
		return;
	}
	_sampleLastCalculated = _m->_currentSample;

	//dependencies
	_noteOnset->calculateNext(); //use default methods

	//no peaks to work with yet
	if (_noteOnset->getPeakHistory()->entries() == 0) {
		return;
	}

	//only calculate in new peaks exist
	if (_noteOnset->getPeakHistory()->newest().onset != _lastPeakOnset) {

		_lastPeakOnset = _noteOnset->getPeakHistory()->newest().onset;

		//new peak => calculate

		if (_noteOnset->getPeakHistory()->entries() < 5) {
			return;
		}

		//do dixon alg
		if (!_initialCalculated) {
			_initialCalculated = true;
			initialDixonAlg();
		}
		else {
			continuousDixonAlg();
		}
		_agents->calculateScoresAccountingForClusterIntervalScores(_clusters);
		_agents->sortAgentsByScoresAccountingForIntervalScores();
		_agents->shrinkSetToSize(40); //only keep this many best agents after every round
		_agents->calculateConfidenceInBestAgent(_clusters->_clusterRadius);
		_agents->devalueScores(0.954); //multiply each agent score by this factor. (0.954 means 15 beats to half)

		//add to rolling avgs
		float tempo = 60.0f * float(_m->_sampleRate) / float(_agents->_highestScoringAgent->_beatInterval);
		_tempoRollingAvg.add(tempo);

		_confidenceRollingAvg.add(_agents->_confidenceInBestAgent);
	}

	//add values to histories, do every call if initialised
	if (_initialCalculated) {
		_tempoHistory.add(_tempoRollingAvg.get(), _m->_currentSample);

		//predictions calc
		int samplesSinceLastBeat = (_m->_currentSample - _agents->_highestScoringAgent->_lastEvent.onset) % _agents->_highestScoringAgent->_beatInterval;
		float timeSinceLastBeat = float(samplesSinceLastBeat) / float(_m->_sampleRate);
		_timeSinceLastBeatHistory.add(timeSinceLastBeat);
		float timeToNextBeat = float(_agents->_highestScoringAgent->_beatInterval - samplesSinceLastBeat) / float(_m->_sampleRate);
		_timeToNextBeatHistory.add(timeToNextBeat);

		//add confidence value (can improve confidence calc later pretty bad atm)
		_confidenceHistory.add(_confidenceRollingAvg.get(), _m->_currentSample);
	}
}

//*** dixon algorithm ***

//relationship function
const float MIN_TEMPO = 30;
const float MAX_TEMPO = 240;
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

//returns false if interval too big too small (implies tempo out of range)
bool intervalImpliesValidTempo(int interval, int sampleRate) {
	if (interval >= (60.0f / MAX_TEMPO) * sampleRate &&
		interval <= (60.0f / MIN_TEMPO) * sampleRate) {
		return true;
	}
	return false;
}


const float CLUSTER_RADIUS_SECONDS = 0.025; //allowed error from average ioi of cluster to be added
const int MAX_PEAKS_STORED = 30; //longer => worse perfomance, more data to use

void TempoDetection::initialDixonAlg() {

	_peaks = _noteOnset->getPeakHistory()->getAsVector(false, 15); //get oldest first, important for agent alg, max last 15 peaks as to

	//*** tempo induction ***
	int clusterRadius = int(CLUSTER_RADIUS_SECONDS * _m->_sampleRate); //need radius in samples

	_clusters = new ClusterSet(clusterRadius);
	computeClusters(_clusters, _peaks);

	//*** phase induction ***
	_agents = new AgentSet(_m->_sampleRate);

	for (auto rit = _clusters->set.rbegin(); rit != _clusters->set.rend(); ++rit) {
		for (auto& peak : _peaks) {
			_agents->add((*rit)._avgInterval, peak);
		}
	}
	computeAgents(_agents, _peaks);
}

void TempoDetection::continuousDixonAlg()
{
	//remove oldest add newest (still oldest first)
	if (_peaks.size() >= MAX_PEAKS_STORED) {
		_peaks.erase(_peaks.begin());
	}
	_peaks.push_back(_noteOnset->getPeakHistory()->newest());

	//*** tempo induction ***
	_clusters->reset(); //delete old clusters
	computeClusters(_clusters, _peaks); //recompute clusters with new peak

	//*** phase induction ***

	//only pass single new peak to agents to update based on the new info
	std::vector<Peak> single;
	single.push_back(_peaks.back());

	for (auto rit = _clusters->set.rbegin(); rit != _clusters->set.rend(); ++rit) { //top ten times only
		_agents->add((*rit)._avgInterval, single.back());
	}

	computeAgents(_agents, single);
}

void TempoDetection::computeClusters(ClusterSet* clusters, std::vector<Peak>& peaks) {

	//compare intervals off all peaks
	for (auto& E1 : peaks) {
		for (auto& E2 : peaks) {
			if (E1.onset == E2.onset) { break; }

			int ioi = fabsf(E1.onset - E2.onset);
			//only add cluster if interval would put tempo as >MIN_TEMPO && <MAX_TEMPO
			if (intervalImpliesValidTempo(ioi, _m->_sampleRate)) {
				clusters->add(ioi); //handles which cluster its added to
			}
		}
	}

	//clusters.debug2();
	//system("PAUSE");

	//merge clusters with average interval below cluster width
	clusters->mergeNearbyClusters();

	//score clusters
	for (auto rit = clusters->set.rbegin(); rit != clusters->set.rend(); ++rit) {
		//set base score depends on how many intervals in cluster
		(*rit)._score += f(1) * (*rit)._IOIs.size();

		for (auto it = clusters->set.begin(); std::next(it) != rit.base(); ++it) {
			//test if itBelow interval factor of it
			for (int k = 2; k <= TEST_RATIOS_UP_TO; k++) {
				//= it.interval - (k * itBelow.interval)
				float difference = (*rit)._avgInterval - (k * ((*it)._avgInterval));

				//if |it.interval - k*itBelow.interval| < clusterRadius
				if (fabsf(difference) <= clusters->_clusterRadius) {
					(*it)._score += f(k) * (*rit)._IOIs.size();
				}
			}
		}
	}

}

void TempoDetection::computeAgents(AgentSet* agents, std::vector<Peak>& peaks)
{
	agents->sortAgentsByPrediction(); //agents sorted by score so need to be fully resorted. lots of errors so dont use insertion

	float tolPrePercentage = 0.2; //20%
	float tolPostPercentage = 0.4; //40%

	int maxTolPre = tolPrePercentage * (60 / MIN_TEMPO) * _m->_sampleRate; //used for optimisation

	int tolInner = 0.05 * _m->_sampleRate; // 50 ms

	int timeOut = 8 * (60 / MIN_TEMPO) * _m->_sampleRate; //how long after last peak that aligns with beat hypothesis before deleteing that agent, 

	float correctionFactor = 0.2; //how fast tempo changes to incorporate new beat information, 0->1, 1 instant, 0 not at all
	int tolPre, tolPost;

	//loop through peaks. must be sorted by increasing event time to stop agent predictions being increased too much immediantly
	for (auto& peak : peaks) {
		//std::cout << "peak: " << peak.onset << std::endl;
		for (auto A = agents->set.begin(); A != agents->set.end(); ) {
			tolPre = tolPrePercentage * (*A)._beatInterval;
			tolPost = tolPostPercentage * (*A)._beatInterval;
			//std::cout << "tol pre " << tolPre << " post " << tolPost << std::endl;


			if (peak.onset - (*A)._lastEvent.onset > timeOut) {
				if (&(*A) == _agents->_highestScoringAgent) {
					std::cout << "OH NO" << std::endl;
				}
				//after specified time passes since last matching beat then discontinue this agent
				//std::cout << (*A)._lastEvent.onset << " <- last event TIMEOUT" << std::endl;
				auto nextA = std::next(A);
				agents->set.erase(A);
				A = nextA;
			}
			else if ((*A)._prediction - maxTolPre > peak.onset) {
				//since agents are in order of prediction & tolPre < maxTolPre we know every agent after this will also have a prediction - tolPre that ignores this event
				//std::cout << "ignore all past this" << std::endl;

				break;
			}
			else {

				while ((*A)._prediction + tolPost < peak.onset) {
					(*A)._prediction += (*A)._beatInterval;
				}
				//std::cout << "agent pred " << (*A)._prediction << " peak " << peak.onset << " interval " << (*A)._beatInterval;

				if ((*A)._prediction - tolPre <= peak.onset && (*A)._prediction + tolPost >= peak.onset) {
					if (SDL_abs((*A)._prediction - peak.onset) > tolInner) { //outside in tolerance but inside outer tolerance
						agents->storeUnmodifiedVersionOfAgent(A); //create new agent that does not count this peak as beat time to protect against wrong decision
					}
					//std::cout << " in outer";

					//count this peak as a beat time
					int error = peak.onset - (*A)._prediction;
					float relativeError = (error > 0 ? float(error) / float(tolPost) : -float(error) / float(tolPre)); //error from 0 -> 1 depending on how close to tolerance limits on either side

					//std::cout << " ,relative error: " << relativeError;

					(*A)._beatInterval += error * correctionFactor;
					(*A)._prediction = peak.onset + (*A)._beatInterval;
					(*A)._lastEvent = peak;
					(*A)._score += (1.0f - (relativeError * 0.5f)) * peak.salience;

				}
				else {
					//std::cout << " ignored";
				}
				++A; //increment
			}
		}
		agents->addBackTheUnmodifiedAgents();
		agents->insertionSortAgentsByPrediction(); //fix any small change in order due to error correction, insertion sort O(n) for minor errors
		agents->removeDuplicateAgents();
	}
}
