#include <string>
#include <fstream>
#include "../include/mcmodm.h"
#include "../include/curl_access.h"
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

DependencyType pb::McModm::McModm::get_dep_type_mr(std::string dep_type){
    if(dep_type=="required")
        return DependencyType::REQUIRED;
    else if(dep_type=="optional")
        return DependencyType::OPTIONAL;
    else if(dep_type=="incompatible")
        return DependencyType::INCOMPATIBLE;
    else if(dep_type=="embedded")
        return DependencyType::EMBEDDED;
    else
        return DependencyType::UNKNOWN;
}
DependencyType pb::McModm::McModm::get_dep_type_cf(int type) {
    switch (type) {
        case 1: return DependencyType::EMBEDDED;
        case 2: return DependencyType::OPTIONAL;
        case 3: return DependencyType::REQUIRED;
        case 5: return DependencyType::INCOMPATIBLE;
        default: return DependencyType::UNKNOWN;
    }
}

std::string pb::McModm::McModm::get_project_name(const std::string& project_id) {
    bool cf_mod = project_id.starts_with("cf:");
    std::string id = cf_mod ? project_id.substr(3) : project_id;
    std::string url = cf_mod?"https://api.curseforge.com/v1/mods/"+id:"https://api.modrinth.com/v2/project/"+id;

    std::string response;
    try {
        if (cf_mod) {
            std::ifstream key(getConfigDir() + "/api_key.txt");
            if (!key.is_open()) 
                throw std::runtime_error("Could not open api_key.txt");
            std::string token;
            std::getline(key, token);
            if (token.empty()) {
                throw std::runtime_error("CurseForge API key is empty");
            }
            response = curl_utils::curl_to_string_with_http_header(url,{"Accept: application/json","x-api-key: " + token},false);
        } else {
            response = curl_utils::curl_to_string(url);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Failed to retrieve project information for " +
            project_id + ": " + e.what()
        );
    }
    try {
        json data = json::parse(response);
        if (cf_mod) {
            return data.at("data").at("name").get<std::string>();
        }
        return data.at("title").get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(
            "Failed to parse project name for " +
            project_id + ": " + e.what()
        );
    }
}

std::string pb::McModm::McModm::dep_type_to_str(DependencyType type) {
    switch (type) {
        case DependencyType::REQUIRED:    return "required";
        case DependencyType::OPTIONAL:    return "optional";
        case DependencyType::INCOMPATIBLE:return "incompatible";
        case DependencyType::EMBEDDED:   return "embedded";
        case DependencyType::UNKNOWN:    return "unknown";
        default:                         return "unknown";
    }
}

void pb::McModm::McModm::set_api_key(std::string key){
    std::string path = pb::McModm::McModm::getConfigDir();
    std::ofstream key_path(path+"/api_key.txt");
    if(!key_path){
        throw std::runtime_error("failed to open key_path.txt");
    }
    key_path<<key;
    key_path.close();
}
std::string pb::McModm::McModm::get_api_key(){
    std::string key="";
    std::string path = pb::McModm::McModm::getConfigDir();
    std::ifstream key_path(path+"/api_key.txt");
    if(!key_path){
        throw std::runtime_error("failed to open key_path.txt");
    }
    key_path>>key;
    return key;
}
void pb::McModm::McModm::remove_api_key() {
    const auto path = std::filesystem::path(getConfigDir()) / "api_key.txt";
    std::error_code ec;
    if (!std::filesystem::remove(path, ec)) {
        if (ec) {
            throw std::runtime_error("failed to remove API key: " + ec.message());
        }
        throw std::runtime_error("API key does not exist");
    }
}
