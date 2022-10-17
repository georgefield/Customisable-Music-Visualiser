#version 330

in vec2 fragmentUV;
in vec2 fragmentPosition;

out vec4 colour;

uniform sampler2D fb1;
uniform float time;

const float scaleFrac = 0.999;
const float recipScaleFrac = 1.0/scaleFrac;
const float border = (1.0-scaleFrac)/2.0;


void main(){

	//colour = texture(fb1, vec2((fragmentUV.x * 2) - 0.5, (fragmentUV.y * 2) - 0.5));
	float offsetX = 0 + sin(5*time) * 0.0002;
	float offsetY = 0.001;

	vec4 shrunkScreen = texture(fb1, vec2((fragmentUV.x - border - offsetX) * recipScaleFrac, (fragmentUV.y - border - offsetY) * recipScaleFrac));
	mat4 colourChangeKernel = mat4(
		0.4,0.2,0.5,0,
		0.2,0.7,0,0,
		0,0.12,0.85,0,
		0,0,0,0
	);
	
	//colour = shrunkScreen;
	if (fragmentUV.x > border + offsetX && fragmentUV.x < (1.0-border) + offsetX
	&&	fragmentUV.y > border + offsetY && fragmentUV.y < (1.0-border) + offsetY){
		colour = 1.01*colourChangeKernel*shrunkScreen;
	}
	else{
		colour = vec4(0,0,0,0);
	}
}