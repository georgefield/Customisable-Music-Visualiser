#pragma once
#include "Master.h"
#include "NoteOnset.h"
#include "RMS.hpp"
#include "Energy.hpp"
#include "TempoDetection.h"
#include "FourierTransform.h"
#include "MFCCs.h"
#include "SelfSimilarityMatrix.h"

#include "AudioManager.h"

#include <GL/glew.h>
#include <unordered_map>

class SignalProcessing {
public:

	static void start();
	static bool ready();

	static void calculate(bool fourierTransform, bool rms, bool energy, bool noteOnset, bool tempoDetection, bool mfccs, bool selfSimilarityMatrix);

	static Master* getMasterPtr() { return _master; }

	static RMS* _rms;
	static Energy* _energy;
	static NoteOnset* _noteOnset;
	static TempoDetection* _tempoDetection;
	static MFCCs* _mfccs;
	static SelfSimilarityMatrix* _selfSimilarityMatrix;

private:
	static Master* _master;

	static bool _started;

	static std::string _currentAudioFilepath;
	static void processChangesToWhatIsCalculated(bool rms, bool energy, bool noteOnset, bool tempoDetection, bool mfccs, bool selfSimilarityMatrix);
};
