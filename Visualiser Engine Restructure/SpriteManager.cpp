#include "SpriteManager.h"

SpriteManager::SpriteManager() :
	_hostWindow(nullptr)
{}

SpriteManager::~SpriteManager()
{
	for (auto& it : _userAddedSprites) { //clean up
		delete it.second;
	}
}

void SpriteManager::init(Vengine::Window* hostWindow)
{
	_hostWindow = hostWindow;
	_spriteBatch.init();
}


void SpriteManager::addSprite(const std::string& name)
{
	auto it = _userAddedSprites.find(name);
	if (it != _userAddedSprites.end()) {
		Vengine::warning("sprite with name '" + name + "' already added");
		return;
	}
	if (_hostWindow == nullptr) {
		Vengine::fatalError("Sprite manager class used without first calling SpriteManager::init");
	}
	_userAddedSprites[name] = new CustomisableSprite(name, _hostWindow);

}

void SpriteManager::deleteSprite(const std::string& name) {
	auto it = _userAddedSprites.find(name);
	if (it != _userAddedSprites.end()) {
		delete it->second;
		_userAddedSprites.erase(name);
	}
}

void SpriteManager::initSprite(const std::string& name, glm::vec2 pos, glm::vec2 dim, float depth, std::string textureFilepath, GLuint glDrawType)
{
	auto it = _userAddedSprites.find(name);
	if (it != _userAddedSprites.end()) {
		it->second->init(pos, dim, depth, textureFilepath, glDrawType);
	}
	else {
		Vengine::warning("sprite with name '" + name + "' does not exist");
	}
}

void SpriteManager::draw()
{
	for (auto& it : _userAddedSprites) {
		it.second->draw();
	}
}

void SpriteManager::drawWithBatching() //useful for fast rendering when not editing
{
	_spriteBatch.begin();

	for (auto& it : _userAddedSprites) {
		_spriteBatch.draw(it.second);
	}
	_spriteBatch.end();

	_spriteBatch.renderBatch();
}

void SpriteManager::processInput(Vengine::InputManager* inputManager)
{
	for (auto& it : _userAddedSprites) {
		it.second->processInput(inputManager);
	}
}

CustomisableSprite* SpriteManager::getSprite(std::string name)
{
	auto it = _userAddedSprites.find(name);
	if (it != _userAddedSprites.end()) {
		return it->second;
	}
	else {
		Vengine::warning("sprite with name '" + name + "' does not exist");
		return nullptr;
	}
}

