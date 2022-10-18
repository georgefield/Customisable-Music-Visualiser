#version 430

in vec2 fragmentUV;
in vec2 fragmentPosition;

out vec4 colour;

uniform sampler2D fb2;

const int n = 4096; //num samples (2 * num frequencies)
const int numFreq = int(n/2);

layout(std430, binding = 1) buffer harmonicValueBuffer
{
	float harmonicValues[]; //between 0 and 1
};


void main(){
	vec4 shrunkScreenCol = texture(fb2, fragmentUV);
	
	float hzFrac = (fragmentPosition.x + 1) * 0.5 ;//* 0.8 + 0.08;
	//hzFrac += 0.1;
	//hzFrac *= 0.85;
	float logHzFrac = pow(2205, hzFrac - 1);//int(hzFrac * hzFrac * hzFrac * numFreq);
	int index = int(logHzFrac * numFreq * 0.7 + 3);
	
	
	if (sqrt(harmonicValues[index] * 2.0f) > (fragmentPosition.y + 1) * 0.5){
		float brightness = (fragmentPosition.y + 1) * 0.5/sqrt(harmonicValues[index]);
		colour = vec4(brightness);// * brightness;
	}
	else{
		colour = shrunkScreenCol;
	}
	
}