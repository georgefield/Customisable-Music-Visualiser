#version 430

// -- WHAT COLOUR TO OUTPUT AT THIS PIXEL --
out vec4 vis_outputColour;
// --


// -- BASE SPRITE INFO --
in vec2 vis_fragmentPosition;
in vec2 vis_fragmentUV;
in vec4 vis_spriteColour;
uniform sampler2D vis_spriteTexture;

//  precomputed input colour for easy use of sprite texture and colour attributes
vec4 vis_inColour = texture(vis_spriteTexture, vis_fragmentUV) * mat4(
																		vis_spriteColour.r, 0, 0, 0,
																		0, vis_spriteColour.g, 0, 0,
																		0, 0, vis_spriteColour.b, 0,
																		0, 0, 0, vis_spriteColour.a
																	);
// --


// -- FOURIER TRANSFORM INFORMATION --
//  contains the frequency magnitude of every harmonic in the fourier transform
//  harmonics are between 0 and 1 but can clip                    vvvv  variable names
layout(std430, binding = 5) buffer FTmaster_harmonics { float vis_FTmaster_harmonics[]; };
layout(std430, binding = 0) buffer FT0_harmonics { float vis_FT0_harmonics[]; };
layout(std430, binding = 1) buffer FT1_harmonics { float vis_FT1_harmonics[]; };
layout(std430, binding = 2) buffer FT2_harmonics { float vis_FT2_harmonics[]; };
layout(std430, binding = 3) buffer FT3_harmonics { float vis_FT3_harmonics[]; };

//  sizes (num harmonics) of each fourier transform
uniform int vis_FTmaster_size;
uniform int vis_FT0_size;
uniform int vis_FT1_size;
uniform int vis_FT2_size;
uniform int vis_FT3_size;
//  energy of each fourier transform, master energy is vis_energy
uniform float vis_FT0_energy;
uniform float vis_FT1_energy;
uniform float vis_FT2_energy;
uniform float vis_FT3_energy;
// --


// -- GENERIC AUDIO & VISUALISER INFO --
//  audio sample data
layout(std430, binding = 4) buffer sampleData { float vis_sampleData[]; };

uniform float vis_timeSinceLoad;
uniform float vis_timeInAudio;
uniform int vis_currentSample;
uniform int vis_totalSamples;
uniform int vis_sampleRate;
// --


// -- VOLUME INFO --
uniform float vis_RMS;
uniform float vis_energy; // <- equivalent to vis_FTmaster_energy
uniform float vis_peakAmplitude;
uniform float vis_peakAmplitudeDb;
// --


// -- NOTE ONSET DETECTION INFO --
uniform float vis_onsetFunctionValue;
uniform float vis_timeSinceLastOnset;
uniform float vis_lastOnsetSalience;
// --


// -- TEMPO INFO --
uniform float vis_tempoEstimate;
uniform float vis_tempoEstimateConfidence;
uniform float vis_timeSinceLastBeat;
uniform float vis_timeToNextBeat;
// --


// -- MFCCs INFO --
layout(std430, binding = 6) buffer melBandEnergies { float vis_melBandEnergies[]; };
layout(std430, binding = 7) buffer melSpectrogram { float vis_melSpectrogram[]; };
layout(std430, binding = 8) buffer MFCCs { float vis_MFCCs[]; };

uniform int vis_numMelBands;
// --


// -- SIMILARITY MATRIX --
uniform sampler2D vis_similarityMatrixTexture;
uniform float vis_similarityMeasure;
uniform float vis_matrixTextureStart;
// --

void main(){
    
    //create useless function that uses all uniforms so they are not optimised away by compiler and names can be fetched
    float useAllFloatAndIntUniformsSoTheyAreRecognised =
    vis_FTmaster_size +
    vis_FT0_size +
    vis_FT1_size +
    vis_FT2_size +
    vis_FT3_size +
    vis_FT0_energy +
    vis_FT1_energy +
    vis_FT2_energy +
    vis_FT3_energy +

    vis_timeSinceLoad +
    vis_timeInAudio +
    vis_currentSample +
    vis_totalSamples +
    vis_sampleRate +

    vis_RMS +
    vis_energy +
    vis_peakAmplitude +
    vis_peakAmplitudeDb +

    vis_onsetFunctionValue +
    vis_timeSinceLastOnset +
    vis_lastOnsetSalience +

    vis_tempoEstimate +
    vis_tempoEstimateConfidence +
    vis_timeSinceLastBeat +
    vis_timeToNextBeat +

    vis_numMelBands +
    vis_similarityMeasure + 
    vis_matrixTextureStart;


    vis_outputColour = vec4(useAllFloatAndIntUniformsSoTheyAreRecognised,1,1,1);
}