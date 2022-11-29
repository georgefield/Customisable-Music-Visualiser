#pragma once
#include <string>

namespace Vengine {

	extern void fatalError(std::string errorMessage);
	extern void warning(std::string warningMessage, bool pause = false);
}