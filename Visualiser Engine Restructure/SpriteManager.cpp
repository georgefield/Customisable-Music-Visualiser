#include "SpriteManager.h"
#include "MyFuncs.h"

SpriteManager::SpriteManager() :
	_viewport(nullptr),
	_window(nullptr),
	_selectedSpriteId(-1)
{
}

SpriteManager::~SpriteManager()
{
	for (auto& it : _userAddedSprites) { //clean up
		delete it.second;
	}
}

void SpriteManager::init(Vengine::Viewport* viewport, Vengine::Window* window)
{
	_window = window;
	_viewport = viewport;
	_spriteBatch.init();
}


void SpriteManager::addSprite(glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType)
{
	//id generation
	int id = _userAddedSprites.size();
	while (_userAddedSprites.find(id) != _userAddedSprites.end()) {
		id++;
	}

	if (_viewport == nullptr) {
		Vengine::fatalError("Sprite manager class used without first calling SpriteManager::init");
	}
	_userAddedSprites[id] = new CustomisableSprite(std::to_string(id), _viewport, _window);

	//init sprite
	_userAddedSprites[id]->init(new Vengine::Quad(), pos, dim, depth, textureFilepath, glDrawType);
}

void SpriteManager::deleteSprite(int id) {
	auto it = _userAddedSprites.find(id);
	if (it != _userAddedSprites.end()) {
		delete it->second; //delete memory of sprite
		_userAddedSprites.erase(it); //delete map entry
	}
}


void SpriteManager::draw()
{
	//then draw all
	for (auto& it : _userAddedSprites) {

		auto shaderProgram = it.second->getShaderProgram();

		shaderProgram->use();

		MyFuncs::setUniformsForShader(shaderProgram);
		it.second->draw();

		shaderProgram->unuse();
	}
}

void SpriteManager::drawWithBatching() //useful for fast rendering when not editing
{
	_spriteBatch.begin();

	for (auto& it : _userAddedSprites) {
		std::cout << "LOL";
		_spriteBatch.draw(it.second);
	}
	_spriteBatch.end();

	_spriteBatch.renderBatch(MyFuncs::setUniformsForShader);
}

void SpriteManager::processInput(Vengine::InputManager* inputManager)
{
	if (inputManager->isChangeThisFrame()) { //only bother checking if change

		//if no sprite selected then see process input for all until one gets selected
		if (_selectedSpriteId == -1) {
			for (auto& it : _userAddedSprites) {
				it.second->processInput(inputManager);
				if (it.second->getSpriteState() == SELECTED) {
					_selectedSpriteId = it.first;
					break;
				}
			}
		}
		//if one gets selected only process input for that sprite
		else {
			_userAddedSprites[_selectedSpriteId]->processInput(inputManager);
			if (_userAddedSprites[_selectedSpriteId]->getSpriteState() == NOT_SELECTED) { //deselected
				_selectedSpriteId = -1;
			}
			else if (_userAddedSprites[_selectedSpriteId]->getSpriteState() == DELETE_SELF) { //delete and deselect
				deleteSprite(_selectedSpriteId);
				_selectedSpriteId = -1;
			}
		}
	}
}
