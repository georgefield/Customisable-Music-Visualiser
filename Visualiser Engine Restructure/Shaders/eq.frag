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

uniform int n;

void main(){
	int numFreq = int(n/2) + 1;
	
	float hzFrac = (fragmentPosition.x + 1) * 0.5;
	float screenFrac = pow(numFreq,hzFrac - 1);
	int index = int(screenFrac * numFreq);
	

	if (harmonicValues[index] > (fragmentPosition.y + 1) * 0.5){
		float brightness = (fragmentPosition.y + 1) * 0.5;
		colour = vec4(1);
	}else{
		colour = vec4(0);
	}
}
