#include "DixonAlgorithmStructures.h"

float DixonAlgVars::CLUSTER_RADIUS_SECONDS = 0.025;
int DixonAlgVars::MAX_PEAKS_STORED = 30;
float DixonAlgVars::SCORE_FACTOR_PER_NEW_BEAT = 0.995;
int DixonAlgVars::MAX_AGENTS_STORED = 40;
float DixonAlgVars::ACCOUNT_FOR_CLUSTER_SCORE = 1.0; //between 0 and 1, 1 is fully and 0 is none