#version 430

in vec2 fragmentPosition;
in vec2 fragmentUV;

out vec4 colour;

const int n = 4096; //num samples (2 * num frequencies)
const int numFreq = int(n/2) + 1;

layout(std430, binding = 0) buffer wavDataBuffer
{
	float wavData[]; //between 0 and 1
};

layout(std430, binding = 1) buffer harmonicValueBuffer
{
	float harmonicValues[]; //between 0 and 1
};


void main(){
	
	float hzFrac = (fragmentPosition.x + 1) * 0.5;
	int index = int(hzFrac * hzFrac * 0.6 * numFreq);

	if (harmonicValues[index] > (fragmentPosition.y + 1) * 0.5){
		float brightness = (fragmentPosition.y + 1) * 0.5;
		colour = vec4(1);
	}else{
		colour = vec4(0);
	}
}
