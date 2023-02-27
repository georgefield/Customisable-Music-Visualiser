#version 430

in vec2 fragmentPosition;
in vec2 fragmentUV;

out vec4 colour;

layout(std430, binding = 0) buffer wavDataBuffer
{
	float wavData[]; //between 0 and 1
};

layout(std430, binding = 1) buffer harmonicValueBuffer
{
	float harmonicValues[]; //between 0 and 1
};

uniform int numHarmonics;
uniform float wagwarn;

void main(){
	
	float numFreq = float(numHarmonics);
	float hzFrac = fragmentUV.x;
	float screenFrac = pow(numFreq,hzFrac - 1);
	int index = int(screenFrac * numFreq);
	

	if (fragmentUV.y < abs(harmonicValues[index])){
		float brightness = fragmentUV.y / abs(harmonicValues[index]);
		if (harmonicValues[index] >= 0){
			colour = vec4(1, 1.0f - wagwarn, 1, 1) * brightness;
		}
		else{
			colour = vec4(1,0,0,1) * brightness;
		}
	}else{
		colour = vec4(0);
	}
}
