#version 330

in vec2 fragmentUV;
in vec2 fragmentPosition;
in vec4 fragmentColour;

out vec4 colour;

uniform float timeSinceLoad;

void main(){

    colour = vec4(cos(0.9 * timeSinceLoad), cos(0.7 * timeSinceLoad), cos(timeSinceLoad), 1);
}