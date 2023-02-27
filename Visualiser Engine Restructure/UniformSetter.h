#pragma once
#include <functional>

template <class T>
struct UniformSetter {

	UniformSetter() : isInitialised(false){}

	void initialiseAsDynamic(std::string functionName, std::function<T()> UpdaterFunction) {
		isInitialised = true;
		functionIsAttached = true;
		isConstant = false;
		updaterFunction = UpdaterFunction;
		setterName = functionName;
		functionValue = NULL; //to set, call 'callUpdater'
	}

	void initialiseAsConstant(std::string constantName, T constantValue) {
		isInitialised = true;
		functionIsAttached = false;
		isConstant = true;
		setterName = constantName;
		functionValue = constantValue;
	}

	bool isValid() {

		if (!isInitialised) {
			Vengine::warning("INVALID SETTER: not initialised");
			return false;
		}

		if (!isConstant && !functionIsAttached) {
			Vengine::warning("INVALID SETTER: not constant or dynamic value");
			return false;
		}

		return true;
	}

	void callUpdater() {
		//function value does not need to be updated if constant
		if (!isInitialised) {
			Vengine::warning("Tried to call updater on uninitiliased uniformSetter");
			return;
		}

		if (isConstant) {
			Vengine::warning("Tried to call updater on constant");
			return;
		}

		if (updaterFunction == nullptr) {
			Vengine::warning("Updater function called without first being set. Setting function value to NULL");
			functionValue = NULL;
		}

		functionValue = updaterFunction();
	}

	bool functionIsAttached;
	bool isConstant;
	bool isInitialised;
	std::string setterName;
	std::function<T()> updaterFunction;
	T functionValue;
};