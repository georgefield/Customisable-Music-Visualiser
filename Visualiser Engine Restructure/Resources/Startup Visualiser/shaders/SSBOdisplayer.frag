#version 430

in vec2 fragmentUV;
in vec2 fragmentPosition;
in vec4 fragmentColour;

out vec4 colour;

layout(std430, binding = 0) buffer theSSBO
{
	float data[]; //should be between 0 and 1
};

uniform sampler2D _texture;

uniform int dataLength;

void main(){
	
    mat4 colourMatrix = mat4(
        fragmentColour.r, 0, 0, 0,
        0, fragmentColour.g, 0, 0,
        0, 0, fragmentColour.b, 0,
        0, 0, 0, fragmentColour.a
    );

    colour = texture(_texture, fragmentUV) * fragmentColour;

	int index = int(fragmentUV.x * dataLength);

	if (fragmentUV.y < abs(data[index])){
		float brightness = fragmentUV.y / abs(data[index]);
        colour *= brightness;

		if (data[index] < 0){
			colour.b = 0; //red for negative (green and blue 0)
            colour.g = 0;
		}
	}else{
		colour = vec4(0);
	}
}
