#pragma once
#include <string>
#include <vector>

class UIglobalFeatures
{
public:
	static void queueError(std::string message);
	static void	displayErrors();

	static std::string ImGuiComboStringMaker(std::vector<std::string>& options);
private:

	static int _errorMessageTimerId;
	static std::vector<std::string> _errorQueue;
};

