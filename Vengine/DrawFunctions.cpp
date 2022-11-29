#include "DrawFunctions.h"
#include "MyErrors.h"

using namespace Vengine;


void DrawFunctions::clearCurrentBuffer() {

	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void DrawFunctions::createDrawBuffers(GLuint* bufferIDs, GLuint* textureIDs, int sizeX, int sizeY, int num) {

	if (num > 5) {
		fatalError("maximum of 5 frame buffers (attachments 0 - 4)");
	}

	glGenFramebuffers(num, bufferIDs);


	// The texture we're going to render to
	glGenTextures(num, textureIDs);
	for (int i = 0; i < num; i++) {
		printf("created frame buffer %u\n", bufferIDs[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, bufferIDs[i]);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, textureIDs[i]);

		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Set "textureID" as our colour attachement #0
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureIDs[i], 0);
		glBindTexture(GL_TEXTURE_2D, 0); //unbind texture id from GL_TEXTURE_2D
	}
		// Set the list of draw buffers.
	GLenum DrawBuffers[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(num, DrawBuffers); // "1" is the size of DrawBuffers

		// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fatalError("frame buffers failed to initiate");
	}
}

void DrawFunctions::setDrawTarget(GLuint bufferID, int sizeX, int sizeY) {

	glBindFramebuffer(GL_FRAMEBUFFER, bufferID);
	glViewport(0, 0, sizeX, sizeY); // Render on the whole framebuffer, complete from the lower left corner to the upper right
}

void DrawFunctions::uploadTextureToShader(GLSLProgram program, GLuint& textureID, const std::string& texVariableName, GLenum tex, int num){

	glActiveTexture(tex);
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLint frameBufferTextureLocationEQ = program.getUniformLocation(texVariableName);
	glUniform1i(frameBufferTextureLocationEQ, num);
}

void DrawFunctions::createSSBO(GLuint& ssboID, GLint binding, void* data, int bytesOfData, GLenum usage) {

	glGenBuffers(1, &ssboID); //generate buffer
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssboID); //make it go in right buffer
	glBufferData(GL_SHADER_STORAGE_BUFFER, bytesOfData, data, usage); //upload normalised data to ssbo
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}

void DrawFunctions::updateSSBO(GLuint& ssboID, GLint binding, void* data, int bytesOfData) {

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, ssboID); //make it go in right buffer
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bytesOfData, data); //upload normalised data to ssbo
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}