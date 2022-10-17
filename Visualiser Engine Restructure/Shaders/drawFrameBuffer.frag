#version 330

in vec2 fragmentUV;
in vec2 fragmentPosition;

out vec4 colour;

uniform sampler2D fb1; 
uniform float time;

void main(){
    vec4 fb1Col = texture(fb1, vec2(fragmentUV.x + 0.01*0.2*(5-sin(time))*cos(fragmentPosition.y * 10),fragmentUV.y));
	//vec4 fb1Col = texture(fb1,fragmentUV);
	colour = fb1Col;
}