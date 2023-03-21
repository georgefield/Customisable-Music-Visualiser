#include "ConfigManager.h"
#include "SPvars.h"
#include "SpriteManager.h"
#include <fstream>
#include <Vengine/MyErrors.h>
#include <Vengine/IOManager.h>
#include "UIglobalFeatures.h"
#include "FourierTransformManager.h"
#include "SignalProcessingManager.h"

//dear programming gods please forgive me for what I have written in this class

bool ConfigManager::outputConfigFromVisualiser(std::string configPath)
{
    Vengine::IOManager::clearFile(configPath); //clear previous config

    std::ofstream out(configPath, std::ios::binary); //start creating new
    if (!out.is_open()) {
        Vengine::warning("Could not open output config file '" + configPath + "'");
        return false;
    }

    //write SPvars struct to file
    out.write(reinterpret_cast<const char*>(&SP::vars), sizeof(SPvarsStruct));

    //write SMinfo struct to file
    out.write(reinterpret_cast<const char*>(&SignalProcessingManager::_similarityMatrix->_SMinfo), sizeof(SimMatInfo));
       
    //write fourier transform data
    char* nullArray = new char[sizeof(FourierTransform::FTinfo)];
    memset(nullArray, NULL, sizeof(FourierTransform::FTinfo));
    for (int id = 0; id < SP::consts._maxFourierTransforms; id++) {
        if (FourierTransformManager::fourierTransformExists(id)){
            auto ft = FourierTransformManager::getFourierTransform(id);
            out.write(reinterpret_cast<const char*>(&ft->_FTinfo), sizeof(FourierTransform::FTinfo));
        }
        else {
            out.write(nullArray, sizeof(FourierTransform::FTinfo)); //write null if not created to keep same size
        }
    }
    delete[] nullArray;

    //write sprite data- variable size so has to be last
    auto spriteMap = SpriteManager::getSpriteMap();

    for (auto& it : *spriteMap) {
        std::cout << it.second->id << " id" << std::endl;
        out.write(reinterpret_cast<const char*>(&it.second->_spriteInfo), sizeof(CustomisableSprite::SpriteInfo));
    }

    out.close();
    return true;
}


bool ConfigManager::initVisualiserFromConfig(std::string configPath)
{
    //open file and do some checks
    if (!Vengine::IOManager::fileExists(configPath)) {
        Vengine::warning("Config file doesnt exist " + configPath);
        UIglobalFeatures::queueError("No config file in visualiser folder (" + configPath + " doesn't exist)");
        return false;
    }

    std::ifstream in(configPath, std::ios::binary | std::ios::ate); //seek to end of file straight away
    if (!in.is_open()) {
        Vengine::warning("Could not open config file");
        UIglobalFeatures::queueError("Could not open config file");
        return false;
    }

    int fileSize = in.tellg(); //get size
    in.seekg(0, std::ios::beg); //go back to beginning
    std::cout << "Check for corruption: " << fileSize << " - " << sizeof(SPvarsStruct) << " - " << sizeof(SimMatInfo) << " - " << SP::consts._maxFourierTransforms << " * " << sizeof(FourierTransform::FTinfo) << " = k *" << sizeof(CustomisableSprite::SpriteInfo) << std::endl;

    if ((fileSize - sizeof(SPvarsStruct) - sizeof(SimMatInfo) - (SP::consts._maxFourierTransforms * sizeof(FourierTransform::FTinfo))) % sizeof(CustomisableSprite::SpriteInfo) != 0) {
        Vengine::warning("Corrupted config file");
        UIglobalFeatures::queueError("Corrupted config file");
        return false;
    }


    //get SPvars data
    unsigned char* SPvarsBuffer = new unsigned char[sizeof(SPvarsStruct)];

    if (!in.read((char*)SPvarsBuffer, sizeof(SPvarsStruct))) {
        Vengine::warning("Could not read SPvars from config file");
        return false;
    }

    SP::vars = *reinterpret_cast<SPvarsStruct*>(SPvarsBuffer); //lol

    delete[] SPvarsBuffer;


    //get SimMat data
    unsigned char* SMinfoBuffer = new unsigned char[sizeof(SimMatInfo)];

    if (!in.read((char*)SMinfoBuffer, sizeof(SimMatInfo))) {
        Vengine::warning("Could not read SPvars from config file");
        return false;
    }

    UIglobalFeatures::_uiSMinfo = *reinterpret_cast<SimMatInfo*>(SMinfoBuffer); //set ui options to match
    SignalProcessingManager::_similarityMatrix->_SMinfo = *reinterpret_cast<SimMatInfo*>(SMinfoBuffer); //set ui options to match
    SignalProcessingManager::_similarityMatrix->reInit(); 

    delete[] SMinfoBuffer;


    //get FT data
    FourierTransformManager::clearFourierTransforms(); //reset fts

    for (int id = 0; id < SP::consts._maxFourierTransforms; id++) {
        unsigned char* FTbuffer = new unsigned char[sizeof(FourierTransform::FTinfo)];

        if (!in.read((char*)FTbuffer, sizeof(FourierTransform::FTinfo))) {
            Vengine::warning("Could not read FT data from config file");
            return false;
        }

        //check if set or not
        bool isNullArray = true;
        for (int i = 0; i < sizeof(FourierTransform::FTinfo); i++) {
            if (FTbuffer[i] != NULL) {
                isNullArray = false;
                break;
            }
        }

        //if set then create transform
        if (!isNullArray) {
            FourierTransform::FTinfo FTinfo = *reinterpret_cast<FourierTransform::FTinfo*>(FTbuffer); //hes done it again that harry kane
            FourierTransformManager::createFourierTransformFromStruct(FTinfo);
            std::cout << id << " ft id" << std::endl;
        }

        delete[] FTbuffer;
    }


    //get sprite data
    SpriteManager::reset(); //remove any existing sprites

    int numSprites = (fileSize - sizeof(SPvarsStruct) - sizeof(SimMatInfo) - (SP::consts._maxFourierTransforms * sizeof(FourierTransform::FTinfo))) / sizeof(CustomisableSprite::SpriteInfo);
    int sizeLeft = fileSize;
    for (int i = 0; i < numSprites; i++){
        unsigned char* spriteBuffer = new unsigned char[sizeof(CustomisableSprite::SpriteInfo)];

        assert(sizeLeft - sizeof(CustomisableSprite::SpriteInfo) >= 0);

        if (!in.read((char*)spriteBuffer, sizeof(CustomisableSprite::SpriteInfo))) {
            Vengine::warning("Could not read sprite data from config file");
            return false;
        }

        CustomisableSprite::SpriteInfo info = *reinterpret_cast<CustomisableSprite::SpriteInfo*>(spriteBuffer); //hes done it again that harry kane
        SpriteManager::addSprite(info);

        delete[] spriteBuffer;
        sizeLeft -= sizeof(CustomisableSprite::SpriteInfo);
    }
}