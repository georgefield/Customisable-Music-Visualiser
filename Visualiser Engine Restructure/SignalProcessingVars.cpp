#include "SignalProcessingVars.h"

//*** UI ***

//which to compute--
bool SPvars::UI::_computeFourierTransform = true;
bool SPvars::UI::_computeRms = true;
bool SPvars::UI::_computeNoteOnset = true;
bool SPvars::UI::_computeTempoDetection = true;
bool SPvars::UI::_computeMFCCs = true;
bool SPvars::UI::_computeSimilarityMatrix = true;
//--

//modifying properties of algorithms--
int SPvars::UI::_onsetDetectionFunctionEnum = 0;
bool SPvars::UI::_convolveOnsetDetection = true;


int SPvars::UI::_nextSimilarityMatrixSize = 100;
bool SPvars::UI::_fastSimilarityMatrixTexture = true;
float SPvars::UI::_similarityMatrixTextureContrastFactor = 20.0f;
int SPvars::UI::_matrixMeasureEnum = 0;
//--

//***


//*** CONST ***

const int SPvars::Const::_generalHistorySize = 1000;

const int SPvars::Const::_numMelBands = 25;

//***