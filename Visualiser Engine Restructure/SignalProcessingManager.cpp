#include "SignalProcessingManager.h"
#include "VisualiserShaderManager.h"
#include "FourierTransformManager.h"

#include <functional>
#include <Vengine/DrawFunctions.h>

Master* SignalProcessing::_master = nullptr;
RMS* SignalProcessing::_rms = nullptr;
Energy* SignalProcessing::_energy = nullptr;
NoteOnset* SignalProcessing::_noteOnset = nullptr;
TempoDetection* SignalProcessing::_tempoDetection = nullptr;
MFCCs* SignalProcessing::_mfccs = nullptr;
SelfSimilarityMatrix* SignalProcessing::_selfSimilarityMatrix = nullptr;

std::string SignalProcessing::_currentAudioFilepath = "";

bool SignalProcessing::_started = false;

void SignalProcessing::start()
{
	if (!ready()) {
		Vengine::warning("Cannot start yet as no audio to play");
		return;
	}

	//if master already exists from previous song then delete it
	if (_master != nullptr) {
		delete _master;
	}

	_master = new Master();
	_master->init(AudioManager::getAudioData(), AudioManager::getSampleRate());

	//set filepath
	_currentAudioFilepath = AudioManager::getAudioFilepath();

	//reset all
	processChangesToWhatIsCalculated(false, false, false, false, false, false);

	_started = true;
}

bool SignalProcessing::ready()
{
	return AudioManager::isAudioLoaded();
}

void SignalProcessing::calculate(bool fourierTransform, bool rms, bool energy, bool noteOnset, bool tempoDetection, bool mfccs, bool selfSimilarityMatrix)
{
	if (_currentAudioFilepath != AudioManager::getAudioFilepath()) { //change in song
		Vengine::warning("Change in audio source, reseting signal processing");
		start(); //reset
	}

	//dependencies
	if (tempoDetection) { noteOnset = true; }

	if (noteOnset) { energy = true; }


	processChangesToWhatIsCalculated(rms, energy, noteOnset, tempoDetection, mfccs, selfSimilarityMatrix);

	if (!AudioManager::isAudioPlaying()) { //no calculations unless audio playing
		return;
	}

	_master->beginCalculations(AudioManager::getCurrentSample());

	//all other signal processing done between begin and end--
	_master->calculateFourierTransform(); //always calculate fourier transform

	if (fourierTransform) {
		FourierTransformManager::calculateFourierTransforms();
	}

	if (rms) {
		_rms->calculateNext(4096, LINEAR_PYRAMID);
	}
	
	if (energy) {
		_energy->calculateNext(4096, LINEAR_PYRAMID);
	}

	if (noteOnset) {
		_noteOnset->calculateNext();
	}

	if (tempoDetection) {
		_tempoDetection->calculateNext();
	}
	//--

	_master->endCalculations();
}


//private

void SignalProcessing::processChangesToWhatIsCalculated(bool rms, bool energy, bool noteOnset, bool tempoDetection, bool mfccs, bool selfSimilarityMatrix)
{
	/// <summary>
	/// on a change of whether a variable is calculated or not create/delete the signal processor object.
	/// ALSO add/delete the uniform & ssbo setter functions
	/// </summary>
	if (rms && _rms == nullptr) {
		_rms = new RMS(400);
		_rms->init(_master);
		std::function<float()> rmsSetterFunc = std::bind(&RMS::getRMS, _rms);
		VisualiserShaderManager::Uniforms::addPossibleUniformSetter("RMS", rmsSetterFunc);

		std::function<float*()> rmsHistorySetterFunc = std::bind(&History<float>::getAsContiguousArray, std::bind(&RMS::getHistory, _rms));
		VisualiserShaderManager::SSBOs::addPossibleSSBOSetter("RMS history", rmsHistorySetterFunc, _rms->getHistory()->totalSize());

		std::function<int()> rmsHistoryLengthSetterFunc = std::bind(&History<float>::totalSize, std::bind(&RMS::getHistory, _rms));
		VisualiserShaderManager::Uniforms::addPossibleUniformSetter("RMS history length", rmsHistoryLengthSetterFunc);
	}
	if (!rms && _rms != nullptr) {
		delete _rms;
		VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("RMS");
		VisualiserShaderManager::SSBOs::deleteSSBOsetter("RMS history");
		VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("RMS history length");
	}


	if (energy && _energy == nullptr) {
		_energy = new Energy(800);
		_energy->init(_master);
		std::function<float()> energySetterFunc = std::bind(&Energy::getEnergy, _energy);
		VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Energy", energySetterFunc);

		std::function<float* ()> energyHistorySetterFunc = std::bind(&History<float>::getAsContiguousArray, std::bind(&Energy::getHistory, _energy));
		VisualiserShaderManager::SSBOs::addPossibleSSBOSetter("Energy history", energyHistorySetterFunc, _energy->getHistory()->totalSize());

		std::function<int()> rmsHistoryLengthSetterFunc = std::bind(&History<float>::totalSize, std::bind(&Energy::getHistory, _energy));
		VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Energy history length", rmsHistoryLengthSetterFunc);
	}
	if (!energy && _energy != nullptr) {
		delete _energy;
		VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("Energy");
		VisualiserShaderManager::SSBOs::deleteSSBOsetter("Energy history");
		VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("Energy history length");
	}


	if (noteOnset && _noteOnset == nullptr) {
		_noteOnset = new NoteOnset(2000);
		_noteOnset->init(_master, _energy);
		std::function<float()> noteOnsetValueSetterFunction = std::bind(&NoteOnset::getOnset, _noteOnset);
		VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Note onset", noteOnsetValueSetterFunction);

		std::function<float* ()> CONVonsetHistorySetterFunction = std::bind(&History<float>::getAsContiguousArray, std::bind(&NoteOnset::getCONVonsetHistory, _noteOnset));
		VisualiserShaderManager::SSBOs::addPossibleSSBOSetter("Note onset history", CONVonsetHistorySetterFunction, _noteOnset->getCONVonsetHistory()->totalSize());

		std::function<int()> CONVonsetHistoryLengthSetterFunc = std::bind(&History<float>::totalSize, std::bind(&NoteOnset::getCONVonsetHistory, _noteOnset));
		VisualiserShaderManager::Uniforms::addPossibleUniformSetter("Note onset history length", CONVonsetHistoryLengthSetterFunc);
	}
	if (!noteOnset && _noteOnset != nullptr) {
		delete _noteOnset;
		VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("Note onset");
		VisualiserShaderManager::SSBOs::deleteSSBOsetter("Note onset history");
		VisualiserShaderManager::Uniforms::deletePossibleUniformSetter("Note onset history length");
	}


	if (tempoDetection && _tempoDetection == nullptr) {
		_tempoDetection = new TempoDetection(2000);
		_tempoDetection->init(_master, _noteOnset);
	}
	if (!tempoDetection && _tempoDetection != nullptr) {
		delete _tempoDetection;
	}
}
