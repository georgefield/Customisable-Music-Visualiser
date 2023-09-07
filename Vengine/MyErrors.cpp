#pragma once
#include "MyErrors.h"

#include <iostream>
#include <SDL/SDL.h>

namespace Vengine {

	void fatalError(std::string errorMessage)
	{
		std::cout << "FATAL: " << errorMessage << std::endl;

		std::cout << "Enter any key to quit: ";
		int tmp;
		std::cin >> tmp;
		SDL_Quit();
		exit(420);
	}

	void warning(std::string warningMessage, bool pause)
	{
		std::cout << "! " << warningMessage << std::endl;

		if (pause) {
			std::cout << "Enter any key to resume: ";
			int tmp;
			std::cin >> tmp;
		}
	}

	void debugMessage(std::string debugMessage)
	{
		std::cout << debugMessage << std::endl;
	}

	void testForGlErrors(std::string messageInCaseOfError) {
		GLenum code = glGetError();
		if (code != GL_NO_ERROR) {
			Vengine::warning(messageInCaseOfError);
		}
		while (code != GL_NO_ERROR) {
			std::cout << " " << code << ": ";
			std::cout << glewGetErrorString(code) << std::endl;
			code = glGetError();
		}
	}
}