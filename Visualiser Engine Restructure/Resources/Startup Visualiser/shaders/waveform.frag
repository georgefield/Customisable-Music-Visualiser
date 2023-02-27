#version 430

in vec2 fragmentPosition;
in vec2 fragmentUV;
in vec4 fragmentColour;

out vec4 colour;

layout(std430, binding = 0) buffer wavDataBuffer
{
	float wavData[]; //between 0 and 1
};

uniform float _time;

void main(){
	int sampleRate = 44100;
	int windowTimeSize = 5;
	
	float hzFrac = fragmentUV.x;
	
	int currentSample = int(_time * float(sampleRate));
	int beepsample = int(hzFrac * float(sampleRate * windowTimeSize)) + currentSample;

	if (abs(fragmentPosition.y - wavData[beepsample]) < 0.1){
		colour = fragmentColour;
	}
	else{
		colour = vec4(0,0,0,0);
	}
}
