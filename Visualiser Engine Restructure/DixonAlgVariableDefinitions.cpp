#include "DixonAlgorithmStructures.h"

float DixonAlgVars::CLUSTER_RADIUS_SECONDS = 0.025;
int DixonAlgVars::MAX_PEAKS_STORED = 20;
float DixonAlgVars::SCORE_FACTOR_PER_NEW_BEAT = 1.0f;
int DixonAlgVars::MAX_AGENTS_STORED = 35;
float DixonAlgVars::ACCOUNT_FOR_CLUSTER_SCORE = 0.0; //between 0 and 1, 1 is fully and 0 is none
float DixonAlgVars::ERROR_CORRECTION_AMOUNT = 0.2;
int DixonAlgVars::NUM_PEAKS_NEEDED_BEFORE_START = 8;
int DixonAlgVars::MAX_AGENT_HISTORY_LENGTH = 80;
float DixonAlgVars::MAX_TIME_PEAKS_SCORED = 20;
float DixonAlgVars::SECONDS_UNTIL_TIMEOUT = 10;