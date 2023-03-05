#include "SignalProcessingVars.h"

//*** UI ***

//which to compute--
bool SPvars::UI::_computeFourierTransform = true;
bool SPvars::UI::_computeRms = true;
bool SPvars::UI::_computeNoteOnset = true;
bool SPvars::UI::_computeTempoDetection = true;
bool SPvars::UI::_computeMFCCs = false;
bool SPvars::UI::_computeSimilarityMatrix = false;
bool SPvars::UI::_computeFutureSimilarityMatrix = false;
//--

//modifying properties of algorithms--
int SPvars::UI::_nextSimilarityMatrixSize = 100;
//--

//***


//*** CONST ***

const int SPvars::Const::_generalHistorySize = 1000;

const int SPvars::Const::_numMelBands = 25;

//***