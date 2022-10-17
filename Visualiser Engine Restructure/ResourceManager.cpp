#include "ResourceManager.h"


TextureCache ResourceManager::_textureCache; //static variables must be declared in cpp file

GLtexture ResourceManager::getTexture(std::string textureFilepath) {
	return _textureCache.getTexture(textureFilepath);
}








int _currentSample;
int _startN;

myComplex complexMulitply(myComplex a, myComplex b) {
	return { a.real * b.real - a.imag * b.imag, a.real * b.imag + a.imag * b.real };
}

myComplex fftRec(int every, int offset, std::vector<float>& samples, int k) {

	int n = _startN / every;

	if (n <= 1) {
		//printf("s[%i]", offset);
		return { samples[offset + _currentSample], 0 };
	}

	float Pi = std::acos(-1);
	myComplex xOddFactor = { std::cosf(-2 * Pi * k * (1.0f / (float)n)), std::sinf(-2 * Pi * k * (1.0f / (float)n)) };
	//printf("(");
	myComplex xEvenPart = fftRec(every * 2, offset, samples, k); //even

	//printf("+e^(-2PIi)k/%i*", n);
	myComplex xOddTmp = fftRec(every * 2, offset + every, samples, k); //odd so add to offset
	myComplex xOddPart = complexMulitply(xOddTmp, xOddFactor);

	//printf(")");
	return { xEvenPart.real + xOddPart.real ,xEvenPart.imag + xOddPart.imag };
}

myComplex fftNonrec(std::vector<float>& samples, int k) { //crap af (undoes any gain (expands brackets))

	float Pi = std::acos(-1);

	myComplex* memArr = new myComplex[(_startN/2)];

	float optMult = -2 * Pi * k * 2.0f * (1.0f/(float)_startN);

	myComplex oddFactor = { cos(optMult * 2), sin(optMult * 2) };
	for (int i = 0; i < _startN / 2; i++) {
		myComplex tmp = complexMulitply({ samples[(2 * i) + 1 + _currentSample], 0 }, oddFactor);
		memArr[i].real = samples[(2 * i) + _currentSample] + tmp.real;
		memArr[i].imag = tmp.imag;
	}
	
	int twoToTheStep = 4;
	int stepNo = 2;
	while (twoToTheStep <= _startN) {
		myComplex oddFactor = { std::cosf(-2 * Pi * k * ((float)twoToTheStep / (float)(2 * _startN))), std::sinf(-2 * Pi * k * ((float)twoToTheStep / (float)(2 * _startN))) };
		for (int i = 0; i < _startN >> stepNo; i++) {
			myComplex tmp = complexMulitply(memArr[(2 * i) + 1], oddFactor);
			memArr[i].real = memArr[(2 * i)].real + tmp.real;
			memArr[i].imag = memArr[(2 * i)].imag + tmp.imag;
		}
		stepNo++;
		twoToTheStep *= 2;   
	}

	myComplex ret = memArr[0];
	delete[] memArr;
	return ret;
}

myComplex ft(std::vector<float>& samples, int k) { //crap af (undoes any gain (expands brackets))

	float Pi = std::acos(-1);

	float fourierIncrement = 1.0f / float(_startN);
	float fourierTime = 0;

	float fourierSinReal;
	float fourierSinImag;

	float dpvx = 0;
	float dpvy = 0;

	for (int s = _currentSample; s < _currentSample + _startN; s++) { //range over samples
		fourierSinReal = cos(-2 * Pi * k * fourierTime); //real part of e^(-2PI * k * (n/N)), fourier time is n/N
		fourierSinImag = sin(-2 * Pi * k * fourierTime); //imag part of ^^^^

		dpvx += fourierSinReal * samples[s];
		dpvy += fourierSinImag * samples[s];

		fourierTime += fourierIncrement;
	}

	return { dpvx, dpvy };
}

void ResourceManager::getFFT(std::vector<float>& samples, int currentSample, std::vector<float>& harmonicValues) {

	int n = harmonicValues.size() * 2;
	_startN = n;
	_currentSample = currentSample;
	for (int k = 0; k < harmonicValues.size(); k++) {
		//myComplex complexRet = ft(samples, k);
		//myComplex complexRet = fftRec(1, 0, samples, k);
		myComplex complexRet = fftNonrec(samples, k);
		harmonicValues.at(k) = sqrtf(complexRet.real * complexRet.real + complexRet.imag * complexRet.imag) / (float)n;
	}

}
