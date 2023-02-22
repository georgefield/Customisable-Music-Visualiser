#version 430

layout(std430, binding = 0) buffer audioDataBuffer
{
	float _audioData[]; //normalised between 0 and 1
};

uniform float _currentSample;
uniform float _sampleRate;

uniform float _rms;
uniform float _energy;

uniform float _noteOnsetValue;
uniform float _timeSinceLastNoteOnset;

uniform float _timeSinceLastBeat;
uniform float _timeToNextBeat;
uniform float _tempo;
uniform float _tempoConfidence;

uniform float _selfSimilarity;