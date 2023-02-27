#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>

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
		std::function<void()> uniformUpdater;
		std::vector<TextureBatch> textureBatches;
	};

	struct SpriteAndProgram {
		Sprite* sprite;
		GLSLProgram* program;
		std::function<void()> uniformUpdater;
	};


	class SpriteBatch
	{
	public:
		SpriteBatch();
		~SpriteBatch();

		void init();

		void begin();
		void draw(Sprite* sprite, GLSLProgram* program, std::function<void()> uniformUpdater);
		void end();
		void renderBatch();
	private:
		void createRenderBatches();
		void createVertexArray();


		//comparison function for stable sort in "SortGlyphs()"
		static bool compareFrontToBack(SpriteAndProgram a, SpriteAndProgram b);
		static bool compareBackToFront(SpriteAndProgram a, SpriteAndProgram b);
		static bool compareTexture(SpriteAndProgram a, SpriteAndProgram b);
		static bool compareShader(SpriteAndProgram a, SpriteAndProgram b);
		//sort function
		void sortSprites();

		GLuint _vbo;
		GLuint _vao;
		int _numVerticesToDraw;

		std::vector<SpriteAndProgram> _spritePtrs;

		std::vector<ProgramBatch> _programBatches; //first array is shaders, second array is textures
	};

}