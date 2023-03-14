#include "ConfigManager.h"
#include "SignalProcessingVars.h"
#include <fstream>
#include <Vengine/MyErrors.h>
#include <Vengine/IOManager.h>

static const std::string tf[2] = {"false", "true"};


bool ConfigManager::outputConfigFromVisualiser(std::string configPath)
{
    Vengine::IOManager::clearFile(configPath); //clear previous config

    std::ofstream out(configPath, std::ios::binary); //start creating new

    //write SPvars struct to file
    if (out.is_open()) {
        out.write(reinterpret_cast<const char*>(&SPvars), sizeof(SPvars));
        out.close();
    }
    else {
        Vengine::warning("Could not output SPvars struct to config file.");
        return false;
    }

    //write sprite data
    //todo

    return true;
}


void ConfigManager::inputVisualiserFromConfig(std::vector<std::string>& in) {

}