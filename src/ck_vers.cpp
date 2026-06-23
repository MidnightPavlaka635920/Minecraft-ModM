#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include "nlohmann/json.hpp"
#include "../include/curl_access.h"
// #include "../include/ck_vers.h"
#include "../include/mcmodm.h"
using json = nlohmann::json;

std::vector<std::string> pb::McModm::McModm::list_compatible_versions(std::string project_id){
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
    for (const auto& release : versionData){
        for (const auto& ver: release["game_versions"]){
            if (std::find(compatible_versions.begin(),
              compatible_versions.end(),
              ver.get<std::string>()) == compatible_versions.end()) {
                compatible_versions.push_back(ver.get<std::string>());
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
