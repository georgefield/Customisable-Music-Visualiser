#version 330

in vec2 vertexPosition;
in vec4 vertexColour;
in vec2 vertexUV;

out vec2 fragmentPosition;
out vec2 fragmentUV;

void main(){
	gl_Position.xy = vertexPosition;
	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
	
	fragmentPosition = vertexPosition; //all will be interpolated between vertices
	fragmentUV =  vec2(vertexUV.x,vertexUV.y); //dont need to flip, already flipped
}