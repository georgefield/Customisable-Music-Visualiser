#pragma once
#include <functional>

template <class T>
struct UniformSetter {

	UniformSetter() : isInitialised(false){}

	void initialise(std::string functionName, std::function<T()> UpdaterFunction) {
		isInitialised = true;
		updaterFunction = UpdaterFunction;
		setterName = functionName;
		functionValue = NULL; //to set, call 'callUpdater'
	}

	bool isValid() {

		if (!isInitialised) {
			Vengine::warning("INVALID SETTER: not initialised");
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

		if (updaterFunction == nullptr) {
			Vengine::warning("Updater function called without first being set");
			return;
		}

		functionValue = updaterFunction();
	}

	bool isInitialised;
	std::string setterName;
	std::function<T()> updaterFunction;
	T functionValue;
};