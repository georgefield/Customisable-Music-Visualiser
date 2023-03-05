#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include "SignalProcessingManager.h"
#include <Vengine/IOManager.h>

//visualiser shader managing--
std::unordered_map<std::string, VisualiserShader> VisualiserShaderManager::_shaderCache;
std::string VisualiserShaderManager::_currentVisualiserPath = "";
//--

//ssbo--
std::unordered_map<int, SSBOsetter> VisualiserShaderManager::_setSSBOinfoMap;
std::unordered_map<std::string, SSBOsetter> VisualiserShaderManager::_SSBOsetterMap;
//--

//uniform setter functions--
std::unordered_map<std::string, UniformSetter<float>> VisualiserShaderManager::_floatFunctions;
std::unordered_map<std::string, UniformSetter<int>> VisualiserShaderManager::_intFunctions;
//--

//default fragment shader
static const std::string DEFAULT_FRAGMENT_SHADER_PATH = "Resources/shaders/simple.frag";


//*** Visualiser shader managing ***

VisualiserShader* VisualiserShaderManager::getShader(std::string fragPath) {
	if (_shaderCache.find(fragPath) != _shaderCache.end()) {
		return &_shaderCache[fragPath];
	}

	//if shader from external add to internal shaders folder
	std::string newFragPath = fragPath;
	if (!Vengine::IOManager::isInParentDirectory(VisualiserManager::shadersFolder(), fragPath)) {
		newFragPath = VisualiserManager::shadersFolder() + fragPath.substr(fragPath.find_last_of('/')); //copy to shaders folder of visualiser
		Vengine::IOManager::copyFile(fragPath, newFragPath);
	}
	_shaderCache[newFragPath].init(newFragPath, _currentVisualiserPath);
	std::cout << "Shader " + fragPath + " loaded for first time" << std::endl;

	return &_shaderCache[newFragPath];
}

std::string VisualiserShaderManager::getDefaultFragmentShaderPath()
{
	return DEFAULT_FRAGMENT_SHADER_PATH;
}

void VisualiserShaderManager::getUsableShaderFragPaths(std::vector<std::string>& shaderPaths)
{
	shaderPaths.clear();

	for (auto& it : _shaderCache){
		shaderPaths.push_back(it.first);
	}
}

//***


//*** SSBOs ***

void VisualiserShaderManager::SSBOs::updateDynamicSSBOs()
{
	for (auto& it : _setSSBOinfoMap) {
		if (!it.second.isValid()) {
			Vengine::fatalError("Invalid SSBO" + it.second.name);
		}
		if (it.second.functionIsAttached) {
			it.second.callUpdater();
			Vengine::DrawFunctions::updateSSBO(it.second.id, it.first, it.second.data, it.second.dataLength * sizeof(float));
		}
	}
}

void VisualiserShaderManager::SSBOs::addPossibleSSBOSetter(std::string functionName, std::function<float* ()> function, int dataLength)
{
	if (_SSBOsetterMap.find(functionName) != _SSBOsetterMap.end()) {
		Vengine::warning("SSBO setter function of  name '" + functionName + "' already exists");
		return;
	}

	_SSBOsetterMap[functionName].initialiseAsDynamic(functionName, function, dataLength);
}

void VisualiserShaderManager::SSBOs::addPossibleSSBOSetter(std::string functionName, float* staticData, int dataLength)
{
	if (_SSBOsetterMap.find(functionName) != _SSBOsetterMap.end()) {
		Vengine::warning("SSBO setter function of  name '" + functionName + "' already exists");
		return;
	}

	_SSBOsetterMap[functionName].initialiseAsConstant(functionName, staticData, dataLength);
}

void VisualiserShaderManager::SSBOs::deleteSSBOsetter(std::string functionName)
{
	if (_SSBOsetterMap.find(functionName) != _SSBOsetterMap.end()) {
		Vengine::warning("No SSBO setter with name '" + functionName + "' to delete");
	}

	//unset any ssbo using that setter function
	for (auto& it : _setSSBOinfoMap) {
		if (it.second.name == functionName) {
			unsetSSBO(it.first);
		}
	}
	//then erase setter function
	_SSBOsetterMap.erase(functionName);
	
}


bool VisualiserShaderManager::SSBOs::setSSBO(int bindingId, std::string functionName)
{
	if (!SSBOinitPossible()) {
		return false;
	}

	if (SSBOisSet(bindingId)) {
		Vengine::warning("SSBO with binding " + std::to_string(bindingId) + " already bound");
		return false;
	}

	if (_SSBOsetterMap.find(functionName) == _SSBOsetterMap.end()) {
		Vengine::warning("No setter function of name " + functionName + ". Cannot init SSBO");
		return false;
	}

	SSBOsetter setterTmp = _SSBOsetterMap[functionName];

	if (!setterTmp.isValid()) {
		Vengine::fatalError("Invalid setter " + setterTmp.name);
	}

	GLenum usage = GL_STATIC_COPY;
	if (setterTmp.functionIsAttached) {
		setterTmp.callUpdater();
		usage = GL_DYNAMIC_COPY;
	}

	_setSSBOinfoMap[bindingId] = setterTmp;
	Vengine::DrawFunctions::createSSBO(_setSSBOinfoMap[bindingId].id, bindingId, _setSSBOinfoMap[bindingId].data, _setSSBOinfoMap[bindingId].dataLength * sizeof(float), usage);

	return true;
}

void VisualiserShaderManager::SSBOs::unsetSSBO(int bindingId)
{
	if (!SSBOisSet(bindingId)) {
		Vengine::warning("Cannot delete SSBO with id " + std::to_string(bindingId) + " as it does not exist");
		return;
	}

	_setSSBOinfoMap.erase(bindingId);
}

bool VisualiserShaderManager::SSBOs::SSBOisSet(int bindingId)
{
	return (_setSSBOinfoMap.find(bindingId) != _setSSBOinfoMap.end());
}

const int MAX_BINDINGS = 8;
void VisualiserShaderManager::SSBOs::getAvailiableBindings(std::vector<int>& availiableBindings)
{
	for (int i = 0; i < MAX_BINDINGS; i++) {
		if (_setSSBOinfoMap.find(i) == _setSSBOinfoMap.end()) {
			availiableBindings.push_back(i);
		}
	}
}

void VisualiserShaderManager::SSBOs::getSetBindings(std::vector<int>& setBindings) {

	for (auto& it : _setSSBOinfoMap) {
		setBindings.push_back(it.first);
	}
}

void VisualiserShaderManager::SSBOs::getSSBOsetterNames(std::vector<std::string>& names)
{
	for (auto& it : _SSBOsetterMap) {
		names.push_back(it.first);
	}
}

SSBOsetter VisualiserShaderManager::SSBOs::getSSBOsetter(std::string name)
{
	if (_SSBOsetterMap.find(name) == _SSBOsetterMap.end()) {
		Vengine::warning("Could not find ssbo setter named '" + name + "'");
		return SSBOsetter();
	}
	return _SSBOsetterMap[name];
}

std::string VisualiserShaderManager::SSBOs::getSSBOsetterName(int bindingID)
{
	if (!SSBOisSet(bindingID)) {
		Vengine::warning("SSBO binding " + std::to_string(bindingID) + " not set so cannot return setter name");
		return "";
	}
	return _setSSBOinfoMap[bindingID].name;
}

void VisualiserShaderManager::addHistoryAsPossibleSSBOsetter(std::string historyName, History<float>* history)
{
	std::function<float()> newestValueSetterFunction = std::bind(&History<float>::newest, history);
	VisualiserShaderManager::Uniforms::addPossibleUniformSetter(historyName, newestValueSetterFunction);

	std::function<float* ()> historySetterFunction = std::bind(&History<float>::getAsContiguousArray, history);
	VisualiserShaderManager::SSBOs::addPossibleSSBOSetter(historyName + " history", historySetterFunction, history->totalSize());

	if (history->totalSize() != SPvars::Const::_generalHistorySize) {
		//need to create specific size var if history size different from general size
		std::function<int ()> historySizeSetterFunction = std::bind(&History<float>::totalSize, history);
		VisualiserShaderManager::Uniforms::addPossibleUniformSetter(historyName + " history size", historySizeSetterFunction);
	}
}

void VisualiserShaderManager::deleteHistoryAsPossibleSSBOsetter(std::string historyName)
{
	VisualiserShaderManager::Uniforms::deletePossibleUniformSetter(historyName);

	VisualiserShaderManager::SSBOs::deleteSSBOsetter(historyName + " history");

	VisualiserShaderManager::Uniforms::deletePossibleUniformSetter(historyName + " history size");

}

bool VisualiserShaderManager::SSBOinitPossible()
{
	//add checks using gl functions that there is enough space in memory
	return true;
}


//***


//*** Uniform setter functions *** 

void VisualiserShaderManager::Uniforms::addPossibleUniformSetter(std::string functionName, std::function<float()> function)
{
	if (_floatFunctions.find(functionName) != _floatFunctions.end()) {
		Vengine::warning("Function with name '" + functionName + "' already in uniform updater list. Uniform updater list not changed.");
		return;
	}

	_floatFunctions[functionName].initialise(functionName, function);
}

void VisualiserShaderManager::Uniforms::addPossibleUniformSetter(std::string functionName, std::function<int()> function)
{
	if (_intFunctions.find(functionName) != _intFunctions.end()) {
		Vengine::warning("Function with name '" + functionName + "' already in uniform updater list. Uniform updater list not changed.");
		return;
	}

	_intFunctions[functionName].initialise(functionName, function);
}


void VisualiserShaderManager::Uniforms::deletePossibleUniformSetter(std::string name)
{
	if (_floatFunctions.find(name) != _floatFunctions.end()) {
		_floatFunctions.erase(name);
		return;
	}

	if (_intFunctions.find(name) != _intFunctions.end()) {
		_intFunctions.erase(name);
		return;
	}

	Vengine::warning("Function '" + name + "' not in the uniform updater function map");
	return;
}

void VisualiserShaderManager::Uniforms::getUniformSetterNames(std::vector<std::string>& names)
{
	getIntUniformSetterNames(names);
	getFloatUniformSetterNames(names);
}

void VisualiserShaderManager::Uniforms::getFloatUniformSetterNames(std::vector<std::string>& names)
{
	for (auto& it : _floatFunctions) {
		names.push_back(it.first);
	}
}

void VisualiserShaderManager::Uniforms::getIntUniformSetterNames(std::vector<std::string>& names)
{
	for (auto& it : _intFunctions) {
		names.push_back(it.first);
	}
}

UniformSetter<float> VisualiserShaderManager::Uniforms::getFloatUniformSetter(std::string functionName)
{
	if (_floatFunctions.find(functionName) == _floatFunctions.end()) {
		Vengine::warning("No setter function with name " + functionName);
		return UniformSetter<float>();
	}

	return _floatFunctions[functionName];
}

UniformSetter<int> VisualiserShaderManager::Uniforms::getIntUniformSetter(std::string functionName)
{
	if (_intFunctions.find(functionName) == _intFunctions.end()) {
		Vengine::warning("No setter function with name " + functionName);
		return UniformSetter<int>();
	}

	return _intFunctions[functionName];
}

//***

