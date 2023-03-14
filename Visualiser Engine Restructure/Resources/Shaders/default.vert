#version 430

in vec2 vertexPosition;
in vec4 vertexColour;
in vec2 vertexUV;

out vec2 vis_fragmentPosition;
out vec2 vis_fragmentUV;
out vec4 vis_spriteColour;

void main(){
	gl_Position.xy = vertexPosition;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
	
	vis_fragmentPosition = vertexPosition;
	vis_fragmentUV = vertexUV;
	vis_spriteColour = vertexColour;
}