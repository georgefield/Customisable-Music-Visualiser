#pragma once
#include "Master.h"
#include "NoteOnset.h"
#include "Energy.hpp"
#include "TempoDetection.h"
#include "FourierTransform.h"
#include "MFCCs.h"
#include "SimilarityMatrixHandler.h"
#include "RMS.hpp"
#include "SignalProcessingVars.h"

#include "AudioManager.h"

#include <GL/glew.h>
#include <unordered_map>

class SignalProcessingManager {
public:

	static void start();
	static void restart();
	static bool ready();

	static void calculate();

	static Master* getMasterPtr() { return _master; }

	static RMS* _rms;
	static NoteOnset* _noteOnset;
	static TempoDetection* _tempoDetection;
	static MFCCs* _mfccs;
	static SimilarityMatrixHandler* _similarityMatrix;

	static int getGeneralHistorySize() { return SPvars::Const::_generalHistorySize; }
private:
	static Master* _master;

	static bool _started;

	static std::string _currentAudioFilepath;

	static void initAlgorithmObjects(bool rms, bool noteOnset, bool tempoDetection, bool mfccs, bool similarityMatrix);
};
