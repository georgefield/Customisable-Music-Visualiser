#include "SpriteManager.h"

#include <algorithm>

int SpriteManager::_selectedSpriteId = -1;

Vengine::Viewport* SpriteManager::_viewport = nullptr;
Vengine::Window* SpriteManager::_window = nullptr;

Vengine::SpriteBatch SpriteManager::_spriteBatch;

//sprite containers
std::unordered_map<int, CustomisableSprite*> SpriteManager::_userAddedSpritePtrs; //main container
std::vector<CustomisableSprite*> SpriteManager::_depthSortedSpritePtrs; //work with this when rendering, updates to match main container when 'updateDepthSortSprites' called

GLuint SpriteManager::_vao, SpriteManager::_vbo;

//wipe customisable sprite containers
void SpriteManager::reset()
{
	for (auto& it : _userAddedSpritePtrs) { //clean up
		delete it.second;
	}

	_userAddedSpritePtrs.clear();
	_depthSortedSpritePtrs.clear();
}

void SpriteManager::init(Vengine::Viewport* viewport, Vengine::Window* window)
{
	_window = window;
	_viewport = viewport;
	_spriteBatch.init();
}


void SpriteManager::addSprite(Vengine::ModelType model, glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath)
{
	//id generation
	int id = _userAddedSpritePtrs.size();
	while (_userAddedSpritePtrs.find(id) != _userAddedSpritePtrs.end()) {
		id++;
	}

	if (_viewport == nullptr) {
		Vengine::fatalError("Sprite manager class used without first calling SpriteManager::init");
	}
	_userAddedSpritePtrs[id] = new CustomisableSprite(id, std::to_string(id), _viewport, _window);

	//init sprite
	if (model == Vengine::Mod_Quad)
		_userAddedSpritePtrs[id]->init(new Vengine::Quad(), pos, dim, depth, textureFilepath, GL_DYNAMIC_DRAW);
	if (model == Vengine::Mod_Triangle)
		_userAddedSpritePtrs[id]->init(new Vengine::Triangle(), pos, dim, depth, textureFilepath, GL_DYNAMIC_DRAW);
	if (model == Vengine::Mod_Ring)
		_userAddedSpritePtrs[id]->init(new Vengine::Ring45side(), pos, dim, depth, textureFilepath, GL_DYNAMIC_DRAW);
	if (model == Vengine::Mod_Circle)
		_userAddedSpritePtrs[id]->init(new Vengine::Circle120side(), pos, dim, depth, textureFilepath, GL_DYNAMIC_DRAW);


	//added sprite is the one selected
	deselectCurrent();
	_userAddedSpritePtrs[id]->setIfSelected(true);

	updateDepthSortedSprites();
}

void SpriteManager::deselectCurrent() {
	if (_selectedSpriteId == -1) {
		return;
	}

	_userAddedSpritePtrs[_selectedSpriteId]->setIfSelected(false);
	_selectedSpriteId = -1;
}

void SpriteManager::select(int id)
{
	deselectCurrent();

	if (_userAddedSpritePtrs.find(id) != _userAddedSpritePtrs.end()) {
		_selectedSpriteId = id;
		_userAddedSpritePtrs[id]->setIfSelected(true);
	}
}

void SpriteManager::deleteSprite(int id) {

	
	auto it = _userAddedSpritePtrs.find(id);

	assert(it != _userAddedSpritePtrs.end());

	delete it->second; //delete memory of sprite
	_userAddedSpritePtrs.erase(it); //delete map entry

	//check if selected sprite has been deleted
	if (_userAddedSpritePtrs.find(_selectedSpriteId) == _userAddedSpritePtrs.end()) {
		_selectedSpriteId = -1; //set sprite to not selected
	}

	updateDepthSortedSprites();
}

void SpriteManager::drawAll() {

	drawNoBatching();
}

void SpriteManager::drawNoBatching()
{
	//draw all user added sprites
	for (auto& it : _depthSortedSpritePtrs) {

		if (!it->isDeleted()) {

			auto shaderProgram = it->getVisualiserShader()->getProgram();

			shaderProgram->use();
			it->getVisualiserShader()->updateUniformValues();

			it->draw();

			shaderProgram->unuse();
		}
	}
}

void SpriteManager::drawWithBatching() //useful for fast rendering when not editing
{
	_spriteBatch.begin();

	for (auto& it : _userAddedSpritePtrs) {

		_spriteBatch.draw(it.second, it.second->getVisualiserShader()->getProgram(), std::bind(&VisualiserShader::updateUniformValues, it.second->getVisualiserShader()));
	}
	_spriteBatch.end();

	_spriteBatch.renderBatch();
}

void SpriteManager::processInput(Vengine::InputManager* inputManager)
{
	if (inputManager->isChangeThisFrame()) { //only bother checking if change

		//if no sprite selected then see process input for all until one gets selected
		if (_selectedSpriteId == -1) {
			for (int i = _depthSortedSpritePtrs.size() - 1; i >= 0; i--) { //backwards through depth
				auto it = _depthSortedSpritePtrs[i];
				it->processInput(inputManager);
				if (it->isSelected()) {
					_selectedSpriteId = _depthSortedSpritePtrs[i]->id;
					break;
				}
			}
		}
		//if one gets selected only process input for that sprite
		else {

			if (_userAddedSpritePtrs[_selectedSpriteId]->isDeleted()) { //delete
				deleteSprite(_selectedSpriteId);
				return;
			}

			//check if selected sprite still exists after deletion
			_userAddedSpritePtrs[_selectedSpriteId]->processInput(inputManager);
			if (!_userAddedSpritePtrs[_selectedSpriteId]->isSelected()) { //deselected
				_selectedSpriteId = -1;
			}
		}
	}
}


void SpriteManager::updateDepthSortedSprites()
{
	_depthSortedSpritePtrs.clear();
	_depthSortedSpritePtrs.reserve(_userAddedSpritePtrs.size());

	for (auto& it : _userAddedSpritePtrs) {
		_depthSortedSpritePtrs.push_back(it.second);
	}

	std::sort(_depthSortedSpritePtrs.begin(), _depthSortedSpritePtrs.end(), [](CustomisableSprite* a, CustomisableSprite* b) {
		return (a->getDepth() > b->getDepth());
		}); //sort by depth
}
