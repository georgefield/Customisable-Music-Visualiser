#version 330

in vec2 fragmentUV;
in vec2 fragmentPosition;

out vec4 colour;

uniform vec3 col;

void main(){
    colour = vec4(col.r, col.g, col.b, 1.0);
}