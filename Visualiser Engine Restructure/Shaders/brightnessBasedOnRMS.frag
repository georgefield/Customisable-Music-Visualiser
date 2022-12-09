#version 330

in vec2 fragmentUV;
in vec2 fragmentPosition;

out vec4 colour;

uniform sampler2D tex;
uniform float rms;

void main(){
        colour = texture(tex, fragmentUV) * rms;
}