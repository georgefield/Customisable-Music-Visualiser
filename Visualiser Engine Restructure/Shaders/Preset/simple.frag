#version 330

in vec2 fragmentUV;
in vec2 fragmentPosition;
in vec4 fragmentColour;

out vec4 colour;

uniform sampler2D _texture;

uniform float _time;

void main(){

    mat4 colourMatrix = mat4(
        fragmentColour.r, 0, 0, 0,
        0, fragmentColour.g, 0, 0,
        0, 0, fragmentColour.b, 0,
        0, 0, 0, fragmentColour.a
    );

    colour = texture(_texture, fragmentUV) * fragmentColour;
}