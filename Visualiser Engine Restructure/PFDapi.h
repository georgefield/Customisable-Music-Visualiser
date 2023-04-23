#pragma once
#include <string>
#include <vector>
class PFDapi
{
public:
	static bool folderChooser(std::string message, std::string startPath, std::string& out, bool loadFromOutsideStartPath);
	static bool fileChooser(std::string message, std::string startPath, std::string& out, const std::vector<std::string>& filters, bool loadFromOutsideStartPath);
	static void openAsyncFileExplorer(std::string message, std::string startPath);
	static bool isAsyncFileExplorerAlive();
};

