//generic .vert that just passes info to .frag

#version 330

in vec2 vertexPosition;
in vec4 vertexColour;
in vec2 vertexUV;

out vec2 fragmentPosition;
out vec2 fragmentUV;
out vec4 fragmentColour;

void main(){
	gl_Position.xy = vertexPosition;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
	
	fragmentPosition = vertexPosition; //all will be interpolated between vertices
	fragmentColour = vertexColour;
	fragmentUV = vertexUV;
}