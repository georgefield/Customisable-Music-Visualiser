#include "VisualiserShaderManager.h"
#include "VisualiserManager.h"
#include "SignalProcessingManager.h"
#include "VisVars.h"
#include <Vengine/IOManager.h>

//visualiser shader managing--
std::unordered_map<std::string, VisualiserShader> VisualiserShaderManager::_shaderCache;
std::string VisualiserShaderManager::_currentVisualiserPath = "";
//--

//ssbo--
std::unordered_map<int, SSBOsetter> VisualiserShaderManager::_SSBOupdaterMap;
std::unordered_map<std::string, int> VisualiserShaderManager::_defaultSSBOnamesAndBindings;

//uniforms
std::unordered_map<std::string, UniformSetter<float>> VisualiserShaderManager::_floatUniformUpdaterMap;
std::unordered_map<std::string, UniformSetter<int>> VisualiserShaderManager::_intUniformUpdaterMap;
std::unordered_map<std::string, GLenum> VisualiserShaderManager::_defaultUniformNames;

static std::string _commonShaderPrefix = "";

GLuint VisualiserShaderManager::_userVarsSSBOid;

//*** Visualiser shader managing ***

void VisualiserShaderManager::init()
{
	Vengine::IOManager::readTextFileToString(VisVars::_shaderPrefixPath, _commonShaderPrefix);

	std::string errorOut;
	Vengine::GLSLProgram* prefixProgram = Vengine::ResourceManager::getShaderProgram(VisVars::_commonVertShaderPath, VisVars::_uniformNameScraperShaderPath, errorOut);
	if (errorOut != "")
		Vengine::fatalError(errorOut);

	for (auto& it : *prefixProgram->getUniformNames()) {
		_defaultUniformNames[it] = prefixProgram->getUniformType(it);
	}

	_defaultSSBOnamesAndBindings["vis_sampleData"] = 4;
	_defaultSSBOnamesAndBindings["vis_FTmaster_harmonics"] = 5;
	for (int i = 0; i < 4; i++) {
		_defaultSSBOnamesAndBindings["vis_FT" + std::to_string(i) + "_harmonics"] = i;
	}
	_defaultSSBOnamesAndBindings["vis_melBandEnergies"] = 6;
	_defaultSSBOnamesAndBindings["vis_melSpectrogram"] = 7;
	_defaultSSBOnamesAndBindings["vis_MFCCs"] = 8;

	//initialise user vars SSBO to 0
	float* zeros = new float[VisVars::_availiableUserVars];
	memset(zeros, 0.0f, VisVars::_availiableUserVars * sizeof(float));
	Vengine::DrawFunctions::createSSBO(_userVarsSSBOid, 9, zeros, sizeof(float) * 3, GL_STATIC_COPY);
	delete[] zeros;
}

VisualiserShader* VisualiserShaderManager::getShader(std::string fragPath) {
	if (_shaderCache.find(fragPath) != _shaderCache.end()) {
		return &_shaderCache[fragPath];
	}

	//must be in parent directory
	assert(Vengine::IOManager::isInParentDirectory(VisualiserManager::shadersFolder(), fragPath));

	if (!_shaderCache[fragPath].init(fragPath, _currentVisualiserPath)) {
		_shaderCache.erase(fragPath);
		Vengine::warning("Shader did not compile properly: error in code");
		return nullptr;
	}
	std::cout << "Shader " + fragPath + " loaded for first time" << std::endl;

	return &_shaderCache[fragPath];
}

std::string VisualiserShaderManager::getCommonShaderPrefix()
{
	return _commonShaderPrefix;
}

bool VisualiserShaderManager::createShader(std::string name)
{
	std::string newFileName = VisualiserManager::shadersFolder() + "/" + name + VisVars::_visShaderExtension;
	if (Vengine::IOManager::fileExists(newFileName)) {
		return false;
	}

	return Vengine::IOManager::copyFile(VisVars::_defaultFragShaderPath, newFileName);
}

void VisualiserShaderManager::recompileShader(std::string fragPath) {
	if (_shaderCache.find(fragPath) == _shaderCache.end()) {
		Vengine::warning("No loaded shader named " + fragPath + " to recompile");
		return;
	}

	_shaderCache.erase(fragPath);
	if (!_shaderCache[fragPath].init(fragPath, _currentVisualiserPath)) {
		_shaderCache.erase(fragPath);
		Vengine::warning("Shader did not recompile: error in code");
		return;
	}

	std::cout << "Shader " + fragPath + " recompiled" << std::endl;
}

//***


//*** SSBOs ***

void VisualiserShaderManager::SSBOs::updateDynamicSSBOs()
{
	for (auto& it : _SSBOupdaterMap) {
		if (!it.second.isValid())
			Vengine::fatalError("Invalid SSBO updater " + it.second.name);

		if (it.second.functionIsAttached) {
			it.second.updateData();
			Vengine::DrawFunctions::updateSSBO(it.second.id, it.first, it.second.data, it.second.dataLength * sizeof(float));
		}
	}
}

void VisualiserShaderManager::SSBOs::setSSBOupdater(std::string SSBOname, std::function<float* ()> function, int dataLength)
{
	//check if name of SSBO in prefix
	if (!isVisSSBO(SSBOname)) {
		Vengine::warning("No SSBO with that name in prefix");
		return;
	}

	//check if already has updater
	int binding = _defaultSSBOnamesAndBindings[SSBOname];
	if (inSSBOmap(binding)) {
		Vengine::warning("SSBO setter function of  binding '" + std::to_string(binding) + "' already exists");
		return;
	}

	//initialise ssbo and its updater
	_SSBOupdaterMap[binding].initialiseAsDynamic(SSBOname, function, dataLength);
	initialiseSSBO(binding);
}

void VisualiserShaderManager::SSBOs::setSSBOupdater(std::string SSBOname, float* staticDataPtr, int dataLength)
{
	//check if name of SSBO in prefix
	if (!isVisSSBO(SSBOname)) {
		Vengine::warning("No SSBO with that name in prefix");
		return;
	}

	//check if already has updater
	int binding = _defaultSSBOnamesAndBindings[SSBOname];
	if (inSSBOmap(binding)) {
		Vengine::warning("SSBO setter function of  binding '" + std::to_string(binding) + "' already exists");
		return;
	}

	_SSBOupdaterMap[binding].initialiseAsConstant(SSBOname, staticDataPtr, dataLength);
	initialiseSSBO(binding);
}

void VisualiserShaderManager::SSBOs::initialiseSSBO(int binding)
{
	if (!inSSBOmap(binding)) {
		Vengine::warning("Not SSBO setter for binding " + std::to_string(binding) + " to initialise SSBO with");
		return;
	}

	SSBOsetter* updater = &_SSBOupdaterMap[binding];

	if (!updater->isValid())
		Vengine::fatalError("Invalid SSBO " + updater->name);

	if (updater->functionIsAttached)
		updater->updateData();

	GLenum usage = updater->isConstant ? GL_STATIC_COPY : GL_DYNAMIC_COPY;
	Vengine::DrawFunctions::createSSBO(updater->id, binding, updater->data, updater->dataLength * sizeof(float), usage);
}

void VisualiserShaderManager::SSBOs::removeSSBOupdater(std::string SSBOname)
{
	if (!isVisSSBO(SSBOname)) {
		Vengine::warning("No SSBO " + SSBOname + " in prefix");
		return;
	}

	int binding = _defaultSSBOnamesAndBindings[SSBOname];
	
	if (!inSSBOmap(binding)) {
		Vengine::warning("No updater for SSBO binding " + std::to_string(binding));
		return;
	}

	Vengine::DrawFunctions::deleteSSBO(_SSBOupdaterMap[binding].id, binding);
	_SSBOupdaterMap.erase(binding);
}

void VisualiserShaderManager::SSBOs::getAllSSBOnames(std::vector<std::string>& names)
{
	for (auto& it : _defaultSSBOnamesAndBindings) {
		names.push_back(it.first);
	}
}

std::string VisualiserShaderManager::SSBOs::getSSBOstatus(std::string name, float* data, int* dataLength)
{
	if (!isVisSSBO(name)) {
		Vengine::warning("SSBO '" + name + "' not in prefix");
		return "???";
	}

	int binding = _defaultSSBOnamesAndBindings[name];
	if (!inSSBOmap(binding)) {
		if (data != nullptr && dataLength != nullptr) {
			data = nullptr;
			*dataLength = 0;
		}
		return "Not set (using will return 0)";
	}

	if (data != nullptr && dataLength != nullptr) {
		data = _SSBOupdaterMap[binding].data;
		*dataLength = _SSBOupdaterMap[binding].dataLength;
	}

	return "Set";
}

//***


//*** Uniform setter functions *** 

void VisualiserShaderManager::Uniforms::setUniformUpdater(std::string uniformName, std::function<float()> function)
{
	if (!isVisUniform(uniformName, GL_FLOAT)) {
		Vengine::warning("No float uniform with name '" + uniformName + "' in prefix");
		return;
	}

	if (inUniformMap(uniformName)) {
		Vengine::warning("Uniform with name '" + uniformName + "' already has updater. Uniform updater list not changed.");
		return;
	}

	_floatUniformUpdaterMap[uniformName].initialise(function);
}

void VisualiserShaderManager::Uniforms::setUniformUpdater(std::string uniformName, std::function<int()> function)
{
	if (!isVisUniform(uniformName, GL_INT)) {
		Vengine::warning("No int uniform with name '" + uniformName + "' in prefix");
		return;
	}

	if (inUniformMap(uniformName)) {
		Vengine::warning("Uniform with name '" + uniformName + "' already has updater. Uniform updater list not changed.");
		return;
	}

	_intUniformUpdaterMap[uniformName].initialise(function);
}

void VisualiserShaderManager::Uniforms::removeUniformUpdater(std::string uniformName)
{
	GLenum type = NULL;
	if (inUniformMap(uniformName, &type)) {
		if (type == GL_INT)
			_intUniformUpdaterMap.erase(uniformName);
		else if (type == GL_FLOAT)
			_floatUniformUpdaterMap.erase(uniformName);

		assert(type != NULL);

		return;
	}

	Vengine::warning("No updater to remove for '" + uniformName + "'");


	_intUniformUpdaterMap.erase(uniformName);
}

void VisualiserShaderManager::Uniforms::getAllUniformNames(std::vector<std::string>& names, GLenum type)
{
	if (type != GL_INT) {
		for (auto& it : _floatUniformUpdaterMap) {
			names.push_back(it.first);
		}
	}

	if (type != GL_FLOAT) {
		for (auto& it : _intUniformUpdaterMap) {
			names.push_back(it.first);
		}
	}
}

float VisualiserShaderManager::Uniforms::getUniformValue(std::string uniformName, GLenum type)
{
	if (!isVisUniform(uniformName, type)) {
		if (type == GL_FLOAT)
			Vengine::warning("Float uniform '" + uniformName + "' not in prefix");
		else if (type == GL_INT)
			Vengine::warning("Int uniform '" + uniformName + "' not in prefix");
		else
			Vengine::warning("Strange type uniform '" + uniformName + "' not in prefix");
		return 0.0f;
	}

	GLenum typeOut;
	if (!inUniformMap(uniformName, &typeOut)) {
		Vengine::warning("Uniform '" + uniformName + "' not in map");
		return 0.0f;
	}
	
	if (typeOut == GL_FLOAT)
		return _floatUniformUpdaterMap[uniformName].functionValue;
	else if (typeOut == GL_INT)
		return _intUniformUpdaterMap[uniformName].functionValue;
}

float VisualiserShaderManager::Uniforms::getUniformValue(std::string uniformName, GLenum* type)
{
	if (!isVisUniform(uniformName)) {
		Vengine::warning("Uniform '" + uniformName + "' not in prefix");
		return 0.0f;
	}

	GLenum typeOut;
	if (!inUniformMap(uniformName, &typeOut)) {
		Vengine::warning("Uniform '" + uniformName + "' not in map");
		return 0.0f;
	}

	*type = typeOut;
	if (typeOut == GL_FLOAT)
		return _floatUniformUpdaterMap[uniformName].functionValue;
	else if (typeOut == GL_INT)
		return _intUniformUpdaterMap[uniformName].functionValue;
}

//***


//private

void VisualiserShaderManager::setShaderUniforms(VisualiserShader* shader)
{
	Vengine::GLSLProgram* program = shader->getProgram();
	assert(program->isBeingUsed());

	for (auto& it : *program->getUniformNames()) {
		GLenum type;
		if (isVisUniform(it, &type)) {
			if (type == GL_FLOAT) {
				auto updater = _floatUniformUpdaterMap[it];
				glUniform1f(program->getUniformLocation(it), updater.functionValue);
			}
			else if (type == GL_INT) {
				auto updater = _intUniformUpdaterMap[it];
				glUniform1i(program->getUniformLocation(it), updater.functionValue);
			}
			else if (type != GL_SAMPLER_2D){
				Vengine::warning("Vis uniform '" + it + "' type not float or int");
			}
		}
		else {
			Vengine::warning("Unknown uniform '" + it + "'");
		}
	}
}

void VisualiserShaderManager::updateUniformValuesToOutput()
{
	for (auto& it : _floatUniformUpdaterMap) {
		it.second.callUpdater();
	}

	for (auto& it : _intUniformUpdaterMap) {
		it.second.callUpdater();
	}
}

bool VisualiserShaderManager::inSSBOmap(int binding)
{
	return _SSBOupdaterMap.find(binding) != _SSBOupdaterMap.end();
}

bool VisualiserShaderManager::isVisSSBO(std::string SSBOname)
{
	return _defaultSSBOnamesAndBindings.find(SSBOname) != _defaultSSBOnamesAndBindings.end();
}

bool VisualiserShaderManager::inUniformMap(std::string uniformName, GLenum* type)
{
	if (_floatUniformUpdaterMap.find(uniformName) != _floatUniformUpdaterMap.end()) {
		*type = GL_FLOAT;
		return true;
	}

	if (_intUniformUpdaterMap.find(uniformName) != _intUniformUpdaterMap.end()) {
		*type = GL_INT;
		return true;
	}

	return false;
}

bool VisualiserShaderManager::inUniformMap(std::string uniformName, GLenum type)
{
	if (type != GL_INT) {
		if (_floatUniformUpdaterMap.find(uniformName) != _floatUniformUpdaterMap.end()) {
			return true;
		}
	}

	if (type != GL_FLOAT) {
		if (_intUniformUpdaterMap.find(uniformName) != _intUniformUpdaterMap.end()) {
			return true;
		}
	}

	return false;
}

bool VisualiserShaderManager::isVisUniform(std::string uniformName, GLenum* type)
{
	if (_defaultUniformNames.find(uniformName) == _defaultUniformNames.end()) {
		return false;
	}

	*type = _defaultUniformNames[uniformName];
	return true;
}

bool VisualiserShaderManager::isVisUniform(std::string uniformName, GLenum type)
{
	if (_defaultUniformNames.find(uniformName) == _defaultUniformNames.end()) {
		return false;
	}

	return type == NULL || _defaultUniformNames[uniformName] == type;
}



