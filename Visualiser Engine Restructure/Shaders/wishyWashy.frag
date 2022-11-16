#version 330

in vec2 fragmentUV;
in vec2 fragmentPosition;

out vec4 colour;

uniform sampler2D tex;
uniform float time;

void main(){
	
	vec2 myUV = vec2(fragmentUV.x + cos(fragmentPosition.x + 2*sin(0.5*time)), fragmentUV.y + cos(1.1*(fragmentPosition.y + time)));
    colour = texture(tex, myUV);
}