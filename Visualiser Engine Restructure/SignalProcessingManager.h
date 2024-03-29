#pragma once
#include "Master.h"
#include "NoteOnset.h"
#include "Energy.hpp"
#include "TempoDetection.h"
#include "FourierTransform.h"
#include "MFCCs.h"
#include "SimilarityMatrixHandler.h"
#include "VisVars.h"
#include "RollingAverage.h"

#include "AudioManager.h"

#include <GL/glew.h>
#include <unordered_map>

class SignalProcessingManager {
public:

	static void calculate(); //call every frame
	static void init();
	static void reset();

	static Master* getMasterPtr() { return _master; }

	static NoteOnset* _noteOnset;
	static TempoDetection* _tempoDetection;
	static MFCCs* _mfccs;
	static SimilarityMatrixHandler* _similarityMatrix;

	static int getGeneralHistorySize() { return Vis::consts._generalHistorySize; }

private:
	static Master* _master;

	static void initAlgorithmObjects(bool noteOnset, bool tempoDetection, bool mfccs, bool similarityMatrix);
};
