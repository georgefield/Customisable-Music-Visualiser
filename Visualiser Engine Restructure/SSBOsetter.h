#pragma once
#pragma once
#include <string>
#include <functional>
#include <Vengine/MyErrors.h>

struct SSBOsetter {

	SSBOsetter() : isInitialised(false), id(0xffffffff) {}

	void initialiseAsDynamic(std::string functionName, std::function<float* ()> UpdaterFunction, int DataLength) {
		isInitialised = true;
		functionIsAttached = true;
		isConstant = false;

		name = functionName;
		dataLength = DataLength;
		updaterFunction = UpdaterFunction;
		data = nullptr;
	}

	void initialiseAsConstant(std::string dataName, float* dataValue, int DataLength) {
		isInitialised = true;
		functionIsAttached = false;
		isConstant = true;

		name = dataName;
		data = dataValue;
		dataLength = DataLength;
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

		if (functionIsAttached) {
			try {
				updaterFunction();
			}
			catch (const std::bad_function_call& e) {
				Vengine::warning("INVALID SETTER: bad function call");
				Vengine::warning(e.what());
				return false;
			}
		}

		return true;
	}

	void callUpdater() {
		//function value does not need to be updated if constant
		if (!isValid()) {
			Vengine::warning("Tried to call updater on invalid SSBOinfo object");
			return;
		}

		data = updaterFunction();
	}

	bool functionIsAttached;
	bool isConstant;
	bool isInitialised;

	std::string name;
	std::function<float* ()> updaterFunction;
	float* data;
	int dataLength;

	GLuint id;
};