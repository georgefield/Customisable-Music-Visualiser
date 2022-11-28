#pragma once
#include "Errors.h"

#include <iostream>
#include <SDL/SDL.h>

namespace Vengine {

	void fatalError(std::string errorMessage) {
		std::cout << errorMessage << std::endl;
		std::cout << "Enter any key to quit: ";
		int tmp;
		std::cin >> tmp;
		SDL_Quit();
		exit(420);
	}

	void warning(std::string warningMessage, bool pause)
	{
		std::cout << warningMessage << std::endl;
		if (pause) {
			std::cout << "Enter any key to resume: ";
			int tmp;
			std::cin >> tmp;
		}
	}
}