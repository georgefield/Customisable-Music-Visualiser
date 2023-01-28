#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "GLSLProgram.h"
#include "Vertex.h"
#include "GLtexture.h"
#include "Sprite.h"

namespace Vengine {

	class TextureBatch {
	public:
		TextureBatch(GLuint TextureID, int BatchOffset)
		{
			textureID = TextureID;
			batchOffset = BatchOffset;
			numVertices = 0;
		}

		GLuint textureID;

		//location information relative to contiguousVertexArray buffered in 'createRenderBatches()'
		int batchOffset;
		int numVertices;
	};

	struct ProgramBatch {
		GLSLProgram* program;
		std::vector<TextureBatch> textureBatches;
	};


	class SpriteBatch
	{
	public:
		SpriteBatch();
		~SpriteBatch();

		void init();

		void begin();
		void draw(Sprite* sprite);
		void end();
		void renderBatch(void (*uniformSetterFunction)(GLSLProgram*));
	private:
		void createRenderBatches();
		void createVertexArray();


		//comparison function for stable sort in "SortGlyphs()"
		static bool compareFrontToBack(Sprite* a, Sprite* b);
		static bool compareBackToFront(Sprite* a, Sprite* b);
		static bool compareTexture(Sprite* a, Sprite* b);
		static bool compareShader(Sprite* a, Sprite* b);
		//sort function
		void sortSprites();

		GLuint _vbo;
		GLuint _vao;
		int _numVerticesToDraw;

		std::vector<Sprite*> _spritePtrs;

		std::vector<ProgramBatch> _programBatches; //first array is shaders, second array is textures
	};

}