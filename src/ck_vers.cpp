#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#include "nlohmann/json.hpp"
#include "../include/curl_access.h"
#include <unordered_set>
// #include "../include/ck_vers.h"
#include "../include/mcmodm.h"
using json = nlohmann::json;

std::vector<std::string> pb::McModm::McModm::list_compatible_versions(std::string project_id, const std::string& loader){
    bool cf_mod = project_id.starts_with("cf:");
    if(cf_mod)
        project_id = project_id.substr(3);
    std::vector<std::string> compatible_versions;
    std::string versionURL,url;
    if(!cf_mod){
        url = "https://api.modrinth.com/v2/project/" + project_id;
        versionURL = "https://api.modrinth.com/v2/project/" + project_id+"/version";
    } else{
        versionURL = "https://api.curseforge.com/v1/mods/" + project_id+"/files";
    }
    std::string aboutVersionData,token;
    if(cf_mod){
        #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (appdata == nullptr) {
            throw std::runtime_error("APPDATA environment variable not found");
        }
        if (!appdata) appdata = ".";
        std::string folder = std::string(appdata) + "\\mcmodm\\";
        //filesystem::create_directories(folder);
        std::string key_path = folder + "api_key.txt";

    #else
        const char* home = std::getenv("HOME");
        if (home == nullptr) {
            throw std::runtime_error("HOME environment variable not found");
        }
        if (!home) home = ".";
        std::string folder = std::string(home) + "/.config/mcmodm/";
        //std::filesystem::create_directories(folder);
        std::string key_path = folder + "api_key.txt";
    #endif

        std::ifstream key(key_path);
        if(!key.is_open()){
            throw std::runtime_error("Could not open api_key.txt");
        }
        std::getline(key,token);
    }

    try {
        if(!cf_mod){
            url = curl_utils::curl_to_string(url);
            if(!cf_mod)
                aboutVersionData = curl_utils::curl_to_string(versionURL);
        } else{
            aboutVersionData = curl_utils::curl_to_string_with_http_header(versionURL,{"Accept: application/json","x-api-key: " + token},false);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching versions: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }

    //json versionData = json::parse(aboutVersionData);
    json versionData,projectData;
    try {
        versionData = json::parse(aboutVersionData);
        if(!cf_mod)
            projectData = json::parse(url);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: [" << aboutVersionData << "]\n";
        std::cerr << "Error: " << e.what() << "\n";
        throw std::runtime_error("Failed to parse JSON.");
    }
    if(loader.empty()&&!cf_mod){
        for (const auto& ver : projectData["game_versions"]){
            if (std::find(compatible_versions.begin(),compatible_versions.end(),ver.get<std::string>()) == compatible_versions.end()) {
                compatible_versions.push_back(ver.get<std::string>());
            }
            
        }
        return compatible_versions;
    } else{
        for(const auto& release:(cf_mod?versionData["data"]:versionData)){
            if(loader.empty()){
                for(const auto&gamever:release["sortableGameVersions"]){
                    if(gamever["gameVersion"].empty())
                        continue;
                    if (std::find(compatible_versions.begin(),compatible_versions.end(),gamever["gameVersion"].get<std::string>()) == compatible_versions.end()) {
                        compatible_versions.push_back(gamever["gameVersion"].get<std::string>());
                    }
                }
                continue;}
            bool lm=false;
            if(cf_mod){
                for(const auto&gamVer:release["gameVersions"]){
                    if(pb::McModm::McModm::toLower(gamVer)==loader){lm=true;break;}
                }
                if(!lm)continue;
            }
            for(const auto& local_loader:(cf_mod?release["sortableGameVersions"]:release["loaders"])){
                if(!cf_mod){
                    if(local_loader.get<std::string>() == loader){
                        for(auto& ver:release["game_versions"]){
                            if (std::find(compatible_versions.begin(),compatible_versions.end(),ver.get<std::string>()) == compatible_versions.end()) {
                                compatible_versions.push_back(ver.get<std::string>());
                            }
                        }
                    }
                }else{
                    if(local_loader["gameVersion"].empty())continue; //Doesn't discard ones that matches loader, but discards if the actual version field is empty
                    std::string gamever = local_loader["gameVersion"].get<std::string>();
                    if (std::find(compatible_versions.begin(),compatible_versions.end(),gamever) == compatible_versions.end()) {
                        compatible_versions.push_back(gamever);
                    }

                }
            }
        }
    }
    return compatible_versions;
}

std::vector<std::string> pb::McModm::McModm::list_comp_loaders(std::string& version, std::string& project_id){
    std::vector<std::string> compatible_versions;
    std::string url = "https://api.modrinth.com/v2/project/" + project_id + "/version";
    std::string aboutVersionData;
    try {
        aboutVersionData = curl_utils::curl_to_string(url);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching versions: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }

    //json versionData = json::parse(aboutVersionData);
    json versionData;
    try {
        versionData = json::parse(aboutVersionData);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: [" << aboutVersionData << "]\n";
        std::cerr << "Error: " << e.what() << "\n";
        throw std::runtime_error("Failed to parse JSON.");
    }
    for (const auto& release: versionData){
        for (const auto& ver: release["game_versions"]){
            if (ver.get<std::string>() == version){
                for (const auto& loader: release["loaders"]){
                    if (std::find(compatible_versions.begin(),
                      compatible_versions.end(),
                      loader.get<std::string>()) == compatible_versions.end()) {
                        compatible_versions.push_back(loader.get<std::string>());
                    }
                }
            }
        }
    }
    return compatible_versions;
}

std::vector<version_names_info> pb::McModm::McModm::list_version_nums(const std::string& project_id, const std::string& game_version){
    const std::string url = "https://api.modrinth.com/v2/project/" + project_id + "/version";
    json releases;
    std::cout<<url<<"\n";
    try{
        std::string raw_releases = pb::curl_utils::curl_to_string(url);
        releases = json::parse(raw_releases);
    }catch(const std::exception& e){
        std::cerr <<"Error: "<< e.what()<<"\n";
    }
    std::vector<version_names_info> version_names;
    for(const auto& release:releases){
        std::string ver_string_current = release["version_number"].get<std::string>();
        std::unordered_set<std::string> game_versions;
        for(const auto& game_version_current:release["game_versions"]){
            game_versions.insert(game_version_current.get<std::string>());
        }
        if(!game_version.empty()&&!game_versions.contains(game_version)){continue;}
        version_names.push_back({ver_string_current,game_versions});
    }
    return version_names;
}
