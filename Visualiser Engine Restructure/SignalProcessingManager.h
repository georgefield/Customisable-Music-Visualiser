#pragma once
#include "Master.h"
#include "NoteOnset.h"
#include "Energy.hpp"
#include "TempoDetection.h"
#include "FourierTransform.h"
#include "MFCCs.h"
#include "SelfSimilarityMatrix.h"
#include "RMS.hpp"

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
	static SelfSimilarityMatrix* _selfSimilarityMatrix;

	static int GENERAL_HISTORY_SIZE;

	static bool UI_computeFourierTransform;
	static bool UI_computeRms;
	static bool UI_computeNoteOnset;
	static bool UI_computeTempoDetection;
	static bool UI_computeMFCCs;
	static bool UI_computeSelfSimilarityMatrix;

	static int getGeneralHistorySize() { return GENERAL_HISTORY_SIZE; }
private:
	static Master* _master;

	static bool _started;

	static std::string _currentAudioFilepath;

	static void initAlgorithmObjects(bool rms, bool noteOnset, bool tempoDetection, bool mfccs, bool selfSimilarityMatrix);
};
