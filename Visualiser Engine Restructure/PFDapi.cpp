#include "PFDapi.h"
#include <portable-file-dialogs.h>
#include <Vengine/MyErrors.h>

#include "UIglobalFeatures.h"

bool PFDapi::folderChooser(std::string message, std::string startPath, std::string& out, bool loadFromOutsideStartPath)
{
	// Check that a backend is available
	if (!pfd::settings::available())
	{
		Vengine::fatalError("Portable File Dialogs are not available on this platform");
	}

	std::replace(startPath.begin(), startPath.end(), '/', '\\');

	// Directory selection
	std::string folderPath = pfd::select_folder(message, startPath, pfd::opt::force_path).result();

	if (!loadFromOutsideStartPath && folderPath.size() >= startPath.size() && folderPath.substr(0, startPath.size()) != startPath) {
		UIglobalFeatures::queueError("Cannot load from outside " + startPath);
		return false;
	}

	//standard is '/' instead of '\' throughout code so rereplace
	std::replace(folderPath.begin(), folderPath.end(), '\\', '/');
	out = folderPath;

	return true;
}

bool PFDapi::fileChooser(std::string message, std::string startPath, std::string& out, const std::vector<std::string>& filters, bool loadFromOutsideStartPath)
{
	// Check that a backend is available
	if (!pfd::settings::available())
	{
		Vengine::fatalError("Portable File Dialogs are not available on this platform");
	}

	std::replace(startPath.begin(), startPath.end(), '/', '\\');

	// Directory selection
	auto chosen = pfd::open_file(message, startPath,
		filters
	, pfd::opt::force_path).result();

	if (chosen.size() == 0) {
		return false;
	}
	std::string filePath = chosen.front();

	if (!loadFromOutsideStartPath && filePath.size() >= startPath.size() && filePath.substr(0, startPath.size()) != startPath) {
		UIglobalFeatures::queueError("Cannot load from outside " + startPath);
		return false;
	}

	//standard is '/' instead of '\' throughout code so rereplace
	std::replace(filePath.begin(), filePath.end(), '\\', '/');
	out = filePath;

	return true;
}
