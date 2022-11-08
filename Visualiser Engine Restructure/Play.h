#pragma once
#include "Sprite.h"
#include "GLSLProgram.h"
#include "DropdownMenu.h"
#include "WindowInfo.h"
#include "FFTW.h"

#include <GL/glew.h>
#include <SDL/SDL.h>

class Play
{
public:
	static void init(WindowInfo windowInfo);
	static int processInput();
	static void draw();
private:
	static void initShaders();

	static int _sampleRate;
	static GLuint* _frameBufferIDs;
	static GLuint* _frameBufferTextureIDs;
	static const int _numFrameBuffers = 3;

	static GLuint _ssboWavDataID;
	static GLuint _ssboHarmonicDataID;
	static GLuint _ssboAllocFFTmemID;

	static std::vector<float> _wavData;
	static std::vector<float> _harmonicData;

	static float* _negArr;

	static int _currSample;
	static int _prevSample;
	static float _sampleOffsetToSound;

	static GLSLProgram _eqProgram;
	static GLSLProgram _drawFrameBufferProgram;
	static GLSLProgram _duplicateFrameProgram;
	static GLSLProgram _shrinkScreenProgram;
	static GLSLProgram _noShading;

	static FFTW _fft;

	static int _globalTimer;

	static WindowInfo _windowInfo;
};

