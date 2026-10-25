#include <string>
#include <fstream>
#include "../include/mcmodm.h"
std::string pb::McModm::McModm::getConfigDir(){
    #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (appdata == nullptr) {
            throw std::runtime_error("APPDATA environment variable not found");
        }
        if (!appdata) appdata = ".";
        std::string folder = std::string(appdata) + "\\mcmodm\\";
        //filesystem::create_directories(folder);

    #else
        const char* home = std::getenv("HOME");
        if (home == nullptr) {
            throw std::runtime_error("HOME environment variable not found");
        }
        if (!home) home = ".";
        std::string folder = std::string(home) + "/.config/mcmodm/";
        //std::filesystem::create_directories(folder);
    #endif
    return folder;
}
