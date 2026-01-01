#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "../include/curl_access.h"
#include <cstdio> // for FILE*, popen
//#include <curl/curl.h>
#include "../include/packages.h"
#include "../include/remove.h"
#include "../include/updateall.h"
#include "../include/install.h"
using json = nlohmann::json;

void update_all_packages(std::string& version, std::string& install_path, std::string& loader, json& req) {
    if (req[0]["version"] == version) {
        std::cout << "No version change detected (" << version << "). Skipping update.\n";
        return;
    }
    set_path(install_path);
    json packgs = load_packages();
    size_t packgs_instal = packgs["installed"].size();
    std::cout << "Found " << packgs_instal << " packages to upgrade.\n";
    //std::cout << packgs["installed"].dump(4) << "\n";
    for (auto& [project_id, info] : packgs["installed"].items()) {
        /*std::cout << "Project ID: " << project_id << "\n";
        std::cout << "  File: " << info["file"] << "\n";    
        std::cout << "  Game version: " << info["game_version"] << "\n";
        std::cout << "  Loader: " << info["loader"] << "\n";*/
        if(info["game_version"] == version && info["loader"] == loader){
            std::cout << "Package " << project_id << " is already up to date for version " << version << ". Skipping.\n";
            continue;
        }
        remove_package(project_id, install_path, true);
        json ti;
        ti.push_back({{"version", version}, {"loader", loader}});
        install_mod(project_id, install_path, ti, true);
    }
    std::string req_path = install_path;
    if (!req_path.empty() && req_path.back() != '/')
        req_path += '/';
    req_path += "req.json";
    json req_s;
    req_s.push_back({{"version", version}, {"loader", loader}});
    std::ofstream ofs(req_path);
    if (!ofs.is_open()) {
        std::cerr << "Failed to write req.json to " << req_path << "\n";
        return;
    }
    ofs << req_s.dump(4);
    ofs.close();
    std::cout << "Updated req.json with new version " << version << ".\n";
}