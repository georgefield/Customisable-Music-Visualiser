#pragma once
#include <string>
#include <vector>
#include "SimMatrixStructs.h"

class UIglobalFeatures
{
public:
	static void queueError(std::string message);
	static void	displayErrors();

	static std::string ImGuiComboStringMaker(std::vector<std::string>& options);
	static bool ImGuiBetterCombo(std::vector<std::string>& options, int& currentItem, int id);
	static bool ImGuiBetterCombo(std::vector<int>& options, int& currentItem, int id);

	static void addSyntaxErrorToWindow(const std::string& error);
	static void clearSyntaxErrorWindow();
	static std::vector<std::string>* getSyntaxErrorArray();

	static SimMatInfo _uiSMinfo;
	static bool _showUI;
private:

	static int _errorMessageTimerId;
	static std::vector<std::string> _errorQueue;
	static std::vector<std::string> _syntaxErrorArray;
};

