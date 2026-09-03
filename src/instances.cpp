#include <string>
#include <nlohmann/json.hpp>
#include "../include/mcmodm.h"
#include<fstream>
#include <iostream>
#include <unordered_map>
using json = nlohmann::json;
std::string pb::McModm::McModm::getPath(std::string& pathString, std::string& instanceString){
    if(!pathString.empty()){return pathString;}else if(!instanceString.empty()){return instanceString;}
    std::string default_path = "";
    #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (!appdata) appdata = ".";
        std::string folder = std::string(appdata) + "\\mcmodm\\";
        //filesystem::create_directories(folder);
        std::string config_path = folder + "defpath.json";

    #else
        const char* home = std::getenv("HOME");
        if (!home) home = ".";
        std::string folder = std::string(home) + "/.config/mcmodm/";
        //std::filesystem::create_directories(folder);
        std::string config_path = folder + "defpath.json";
    #endif
    if (std::filesystem::exists(config_path)) {
        std::ifstream config_pt(config_path);
        if (config_pt.is_open()) {
            json config_json;
            config_json = json::parse(config_pt);
            if (config_json.contains("default_path")) {
                return config_json["default_path"];
                //std::cout << "Default path loaded from config: " << default_path << "\n";
            } else {
                std::cerr << "Config file does not contain 'default_path'.\n";
            }
        }
    }

return "";
}

std::unordered_map<std::string, std::string> getInstances(){
    std::unordered_map<std::string, std::string> allInstances;
    std::string default_path = "";
    #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (!appdata) appdata = ".";
        std::string folder = std::string(appdata) + "\\mcmodm\\";
        //filesystem::create_directories(folder);
        std::string config_path = folder + "instances.json";

    #else
        const char* home = std::getenv("HOME");
        if (!home) home = ".";
        std::string folder = std::string(home) + "/.config/mcmodm/";
        //std::filesystem::create_directories(folder);
        std::string config_path = folder + "instances.json";
    #endif
    if (std::filesystem::exists(config_path)) {
        std::ifstream config_pt(config_path);
        if (config_pt.is_open()) {
            json config_json;
            config_json = json::parse(config_pt);
            if (config_json.contains("instances")) {
                for(const auto& curInstance:config_json["instances"]){
                    allInstances[curInstance["name"]] = curInstance["path"];
                }
            } else {
                std::cerr << "Config file does not contain 'default_path'.\n";
            }
        }
    }
return allInstances;
}
