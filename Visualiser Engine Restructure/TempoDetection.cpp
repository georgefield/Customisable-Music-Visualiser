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

	//no peaks to work with yet
	if (_noteOnset->getPeakHistory()->entries() == 0) {
		return;
	}

	//beat times need to be updated every calculation cycle if best agent variables set
	if (_agents->bestAgentVarsSet()) {
		int samplesSinceLastBeat = (_m->_currentSample - _agents->_bestAgentBeatInterval) % _agents->_bestAgentBeatInterval;
		float timeSinceLastBeat = float(samplesSinceLastBeat) / float(_m->_sampleRate);
		_timeSinceLastBeat = timeSinceLastBeat;

		float timeToNextBeat = float(_agents->_bestAgentBeatInterval - samplesSinceLastBeat) / float(_m->_sampleRate);
		_timeToNextBeat = timeToNextBeat;
	}

	//only calculate if new peaks exist
	if (_noteOnset->getPeakHistory()->newest().onset == _lastPeakOnset) {
		return;
	}

	_lastPeakOnset = _noteOnset->getPeakHistory()->newest().onset;

	//only calculate if enough peaks
	if (_noteOnset->getPeakHistory()->entries() < DixonAlgVars::NUM_PEAKS_NEEDED_BEFORE_START) { //8 onsets atleast before starting to calculate
		_confidenceRollingAvg.add(0.0f); //want to start confidence as low
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

	_agents->calculateScoresAndSetBestAgent(_clusters);
	_agents->sortAgentsByScores();
	_agents->shrinkSetToSize(DixonAlgVars::MAX_AGENTS_STORED); //only keep this many best agents after every round
	_agents->calculateConfidenceInBestAgent(_clusters->_clusterRadius * 2); //2 * cluster radius
	_agents->devalueScores(DixonAlgVars::SCORE_FACTOR_PER_NEW_BEAT); //multiply each agent score by this factor. (0.954 means 15 beats to half)

	//only set best agents vars if found a best agent
	if (!_agents->bestAgentVarsSet()) {
		return;
	}

	//add to rolling avgs
	float tempo = 60.0f * float(_m->_sampleRate) / float(_agents->_bestAgentBeatInterval);

	//set tempo and confidence values
	_tempoRollingAvg.add(tempo);
	_confidenceRollingAvg.add(_agents->_confidenceInBestAgent);

	_tempo = _tempoRollingAvg.get();
	_tempoConfidence = _confidenceRollingAvg.get();
}


//*** dixon algorithm ***

//relationship function

const int TEST_RATIOS_UP_TO = 8;
int f(int d) {
	if (d > 8)
		return 0;
	if (d > 4)
		return 1;
	if (d == 4)
		return 4;
	if (d == 3)
		return 4;
	if (d == 2)
		return 5;
	if (d == 1)
		return 8;
	Vengine::warning("negative d value for relationship function");
	return 0;
}


void TempoDetection::initialDixonAlg() {

	_peaks.clear();
	int i = 0;
	while (i < DixonAlgVars::MAX_PEAKS_STORED && i < _noteOnset->getPeakHistory()->entries()){
		_peaks.push_back(_noteOnset->getPeakHistory()->get(i)); //oldest first, important for agent alg
		i++;
	}

	//*** tempo induction ***

	computeClusters(_clusters, _peaks);

	//*** phase induction ***

	for (auto rit = _clusters->set.rbegin(); rit != _clusters->set.rend(); ++rit) {
		for (auto& peak : _peaks) {
			if (DixonAlgFunc::intervalImpliesValidTempo((*rit)._avgInterval, _m->_sampleRate)) {
				_agents->add((*rit)._avgInterval, peak);
			}
		}
	}

	computeAgents(_agents, _peaks);
}

void TempoDetection::continuousDixonAlg()
{
	//remove oldest add newest (still oldest first)
	while (_peaks.size() >= DixonAlgVars::MAX_PEAKS_STORED) {
		_peaks.erase(_peaks.begin());
	}
	_peaks.push_back(_noteOnset->getPeakHistory()->newest()); //time and salience

	//*** tempo induction ***
	_clusters->reset(); //delete old clusters
	computeClusters(_clusters, _peaks); //recompute clusters with new peak

	//*** phase induction ***

	//only pass single new peak to agents to update based on the new info
	std::vector<Peak> single;
	single.push_back(_peaks.back());

	for (auto rit = _clusters->set.rbegin(); rit != _clusters->set.rend(); ++rit) {
		if (DixonAlgFunc::intervalImpliesValidTempo((*rit)._avgInterval, _m->_sampleRate)) {
			_agents->add((*rit)._avgInterval, single.back());
		}
	}

	computeAgents(_agents, single);
}

void TempoDetection::computeClusters(ClusterSet* clusters, std::vector<Peak>& peaks) {

	//compare intervals off all peaks
	for (auto& E1 : peaks) {
		for (auto& E2 : peaks) {
			if (E1.onset == E2.onset) { break; }

			int ioi = fabsf(E1.onset - E2.onset);

			clusters->add(ioi); //handles which cluster its added to
		}
	}

	//clusters.debug2();
	//system("PAUSE");

	//merge clusters with average interval below cluster width
	clusters->mergeNearbyClusters();

	//score clusters
	for (auto larger = clusters->set.rbegin(); larger != clusters->set.rend(); ++larger) {
		//set base score depends on how many intervals in cluster
		(*larger)._score += f(1) * (*larger)._IOIs.size();

		for (auto smaller = clusters->set.begin(); std::next(smaller) != larger.base(); ++smaller) {
			//test if itBelow interval factor of it
			for (int k = 2; k <= TEST_RATIOS_UP_TO; k++) {
				//= it.interval - (k * itBelow.interval)
				float difference = (k * ((*smaller)._avgInterval)) - (*larger)._avgInterval;

				//if |it.interval - k*itBelow.interval| < clusterRadius
				if (fabsf(difference) <= clusters->_clusterRadius) {
					(*smaller)._score += f(k) * (*larger)._IOIs.size();
				}
			}
		}
	}

	

}

void TempoDetection::computeAgents(AgentSet* agents, std::vector<Peak>& peaks)
{
	agents->prepareForCompute(); //resort agents by prediction if required

	float tolPrePercentage = 0.2; //20%
	float tolPostPercentage = 0.4; //40%

	int maxTolPre = tolPrePercentage * (60.0f / Vis::vars.MIN_TEMPO) * _m->_sampleRate; //used for optimisation

	int tolInner = 0.025 * _m->_sampleRate; // 25 ms

	int tolPre, tolPost;

	//loop through peaks. must be sorted by increasing event time to stop agent predictions being increased too much immediantly
	for (auto& peak : peaks) {
		//std::cout << "peak: " << peak.onset << std::endl;
		for (auto A = agents->set.begin(); A != agents->set.end(); ++A) {

			tolPre = tolPrePercentage * (*A)->_beatInterval;
			tolPost = tolPostPercentage * (*A)->_beatInterval;
			//std::cout << "tol pre " << tolPre << " post " << tolPost << std::endl;

			if ((*A)->_prediction - maxTolPre > peak.onset) {
				//since agents are in order of prediction & tolPre < maxTolPre we know every agent after this will also have a prediction - tolPre that ignores this event
				//std::cout << "ignore all past this" << std::endl;

				break;
			}
			else {

				while ((*A)->_prediction + tolPost < peak.onset) {
					(*A)->_prediction += (*A)->_beatInterval;
				}
				//std::cout << "agent pred " << (*A)._prediction << " peak " << peak.onset << " interval " << (*A)._beatInterval;

				if ((*A)->_prediction - tolPre <= peak.onset && (*A)->_prediction + tolPost >= peak.onset) {
					if (SDL_abs((*A)->_prediction - peak.onset) > tolInner) { //outside in tolerance but inside outer tolerance
						agents->storeUnmodifiedVersionOfAgent((*A)); //create new agent that does not count this peak as beat time to protect against wrong decision
					}
					//std::cout << " in outer";

					//count this peak as a beat time
					int error = peak.onset - (*A)->_prediction;
					float relativeError = (error > 0 ? float(error) / float(tolPost) : -float(error) / float(tolPre)); //error from 0 -> 1 depending on how close to tolerance limits on either side

					(*A)->add(peak, error, relativeError);
				}
				else {
					//std::cout << " ignored";
				}
			}
		}

		agents->addBackTheUnmodifiedAgents();
		agents->sortAgentsByPrediction();
		agents->removeDuplicateAgents();
	}

	agents->removeTimedOutAgents(peaks.back());
	agents->removeAgentsWithBadTempo();
	agents->removeOldPeaksFromAgents(_m->_currentSample);
}

void TempoDetection::initUpdaters()
{
	std::function<float()> tempoUpdaterFunction = std::bind(&TempoDetection::getTempo, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_tempoEstimate", tempoUpdaterFunction);

	std::function<float()> confidenceInTempoUpdaterFunction = std::bind(&TempoDetection::getConfidenceInTempo, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_tempoEstimateConfidence", confidenceInTempoUpdaterFunction);

	std::function<float()> timeSinceLastBeatUpdaterFunction = std::bind(&TempoDetection::getTimeSinceLastBeat, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_timeSinceLastBeat", timeSinceLastBeatUpdaterFunction);

	std::function<float()> timeToNextBeatUpdaterFunction = std::bind(&TempoDetection::getTimeToNextBeat, this);
	VisualiserShaderManager::Uniforms::setUniformUpdater("vis_timeToNextBeat", timeToNextBeatUpdaterFunction);
}

void TempoDetection::removeUpdaters()
{
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_tempoEstimate");
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_tempoEstimateConfidence");
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_timeSinceLastBeat");
	VisualiserShaderManager::Uniforms::removeUniformUpdater("vis_timeToNextBeat");
}
