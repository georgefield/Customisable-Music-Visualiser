#pragma once
#include <functional>

template <class T>
struct UniformSetter {

	void initialiseAsDynamic(std::string functionName, std::function<T()> UpdaterFunction) {
		isSet = true;
		functionIsAttached = true;
		isConstant = false;
		updaterFunction = UpdaterFunction;
		setterName = functionName;
		functionValue = NULL; //to set, call 'callUpdater'
	}

	void initialiseAsConstant(std::string constantName, T constantValue) {
		isSet = true;
		functionIsAttached = false;
		isConstant = true;
		setterName = constantName;
		functionValue = constantValue;
	}

	void initialiseAsNotSet() {
		isSet = false;
		functionIsAttached = false;
		isConstant = true;
		setterName = "None";
		functionValue = T(NULL);
	}

	bool isValid() {

		if (!isConstant && !functionIsAttached) {
			Vengine::warning("INVALID SETTER: not constant or dynamic value");
			return false;
		}

		return true;
	}

	void callUpdater() {
		//function value does not need to be updated if constant
		if (!isSet) {
			Vengine::warning("Tried to call updater on unset uniformSetter");
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
	bool isSet;
	std::string setterName;
	std::function<T()> updaterFunction;
	T functionValue;
};