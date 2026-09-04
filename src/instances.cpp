#include <string>
#include <nlohmann/json.hpp>
#include "../include/mcmodm.h"
#include<fstream>
#include <iostream>
#include <unordered_map>
using json = nlohmann::json;
std::string pb::McModm::McModm::getPath(std::string& pathString, std::string& instanceString){
    if(!pathString.empty()){return pathString;}else if(!instanceString.empty()){
        const auto& instances = getInstances();
        if(instances.contains(instanceString))
            return instances.at(instanceString);
        return "";
    }
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

std::unordered_map<std::string, std::string> pb::McModm::McModm::getInstances(){
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
void pb::McModm::McModm::addInstance(std::string& path, std::string& name){
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

    std::ifstream insFile(config_path);
        json insParsed;

    if (insFile && insFile.peek() != std::ifstream::traits_type::eof()) {
        insParsed = json::parse(insFile);
    } else {
        insParsed = {
            {"instances", json::object()}
        };
    }
    insFile.close();
    insParsed["instances"][name] ={{"path",path}};
    std::ofstream of(config_path);
    of<<insParsed.dump(4);
}
void pb::McModm::McModm::removeInstance(std::string& name){
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

    //if(!input.is_open()){throw std::runtime_error("could not open instance file\n");}
    json insParsed;

    std::ifstream insFile(config_path);
    if (insFile && insFile.peek() != std::ifstream::traits_type::eof()) {
        insParsed = json::parse(insFile);
    } else {
        insParsed = {
            {"instances", json::object()}
        };
    }
    insFile.close();
    insParsed["instances"].erase(name);
    std::ofstream output(config_path);
    output<<insParsed.dump(4);
    output.close();
}
