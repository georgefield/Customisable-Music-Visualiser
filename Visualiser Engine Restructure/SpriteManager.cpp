#include "SpriteManager.h"

#include <algorithm>

int SpriteManager::_selectedSpriteId = -1;

Vengine::Viewport* SpriteManager::_viewport = nullptr;
Vengine::Window* SpriteManager::_window = nullptr;

Vengine::SpriteBatch SpriteManager::_spriteBatch;

//sprite containers
std::unordered_map<int, CustomisableSprite*> SpriteManager::_userAddedSprites; //main container
std::vector<CustomisableSprite*> SpriteManager::_depthSortedSprites; //work with this when rendering, updates to match main container when 'updateDepthSortSprites' called

GLuint SpriteManager::_vao, SpriteManager::_vbo;

//wipe customisable sprite containers
void SpriteManager::reset()
{
	for (auto& it : _userAddedSprites) { //clean up
		delete it.second;
	}
	
	_userAddedSprites.clear();
	_depthSortedSprites.clear();
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
	_userAddedSprites[id] = new CustomisableSprite(id, std::to_string(id), _viewport, _window);

	//init sprite
	_userAddedSprites[id]->init(new Vengine::Quad(), pos, dim, depth, textureFilepath, glDrawType);

	updateDepthSortedSprites();
}

void SpriteManager::deleteSprite(int id) {

	auto it = _userAddedSprites.find(id);
	if (it != _userAddedSprites.end()) {
		delete it->second; //delete memory of sprite
		_userAddedSprites.erase(it); //delete map entry
	}

	updateDepthSortedSprites();
}


void SpriteManager::drawAll(bool batching) {

	if (batching) {
		drawWithBatching();
	}
	else {
		drawNoBatching();
	}
}

void SpriteManager::drawNoBatching()
{
	//draw all user added sprites
	for (auto& it : _depthSortedSprites) {

		auto shaderProgram = it->getVisualiserShader()->getProgram();

		shaderProgram->use();
		it->getVisualiserShader()->updateUniformValues();
		
		it->draw();

		shaderProgram->unuse();
	}
}

void SpriteManager::drawWithBatching() //useful for fast rendering when not editing
{
	_spriteBatch.begin();

	for (auto& it : _userAddedSprites) {

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
			for (int i = _depthSortedSprites.size() - 1; i >= 0; i--) { //backwards through depth
				auto it = _depthSortedSprites[i];
				it->processInput(inputManager);
				if (it->getSpriteState() == SELECTED) {
					_selectedSpriteId = _depthSortedSprites[i]->id; 
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


void SpriteManager::updateDepthSortedSprites()
{
	_depthSortedSprites.clear();
	_depthSortedSprites.reserve(_userAddedSprites.size());

	for (auto& it : _userAddedSprites) {
		_depthSortedSprites.push_back(it.second);
	}

	std::sort(_depthSortedSprites.begin(), _depthSortedSprites.end(), [](CustomisableSprite* a, CustomisableSprite* b) {
		return (a->getDepth() > b->getDepth());
	}); //sort by depth
}
