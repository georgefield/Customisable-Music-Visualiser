#version 430

in vec2 fragmentPosition;
in vec2 fragmentUV;

out vec4 colour;

const int n = 4096; //num samples (2 * num frequencies)
const int numFreq = int(n/2);

layout(std430, binding = 1) buffer harmonicValueBuffer
{
	float harmonicValues[]; //between 0 and 1
};


void main(){
	
	float hzFrac = (fragmentPosition.x + 1) * 0.5;
	int index = int(hzFrac * 0.6 * hzFrac * numFreq);

	if (sqrt(harmonicValues[index] * 3.0f) > (fragmentPosition.y + 1) * 0.5){
		float brightness = (fragmentPosition.y + 1) * 0.5/sqrt(harmonicValues[index] * 2.0f);
		colour = vec4(1,1,1,1) * brightness;
	}
}
