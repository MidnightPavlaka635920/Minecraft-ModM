#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <cstdio> // for FILE*, popen
//#include <curl/curl.h>
#include "../include/mcmodm.h"
#include "../include/color.h"
/*
#include "../include/packages.h"
#include "../include/remove.h"
#include "../include/updateall.h"
#include "../include/install.h"
*/
using json = nlohmann::json;
#include <unordered_set>
void pb::McModm::McModm::update_all_packages(std::string& version, std::vector<std::string>& loaders, json& req, bool force) {
    if (req[0]["version"] == version&&!force) {
        std::cout << "No version change detected (" << version << "). Skipping update.\n";
        return;
    }
    //set_path(install_path);
    json packgs = load_packages();
    size_t packgs_instal = packgs["installed"].size();
    std::cout << "Found " << packgs_instal << " packages to upgrade.\n";
    //std::cout << packgs["installed"].dump(4) << "\n";
    bool all_updatable = true;
    //std::vector<bool> upgradibleList;
    std::unordered_set<std::string> upgradeableMods;
    for (auto& [project_id, info] : packgs["installed"].items()) {
        bool upgradable = false;
        if (project_id.starts_with("local:")){
            std::cout << "\033[33mPackages installed from a file will not be updated.\033[0m\n";
            continue;
        } else{
            upgradable = can_be_upgraded(project_id, version, info["loader"]);
            if (force&&upgradable){upgradeableMods.insert(project_id);}
            if (!upgradable) {
                all_updatable = false;
                //std::cout << "Package " << project_id << " cannot be upgraded to version " << version << " with info[loader] " << info[loader] << ".\n";
                //continue;
            }
            }
            std::cout << "Is " << info["name"].get<std::string>() <<" (" <<project_id<< ") upgradable to version " << version << ": "<<(upgradable ? "\033[32mYes\033[0m" : "\033[31mNo\033[0m") << ".\n";
        
    }
    if (!all_updatable) {
        if (!force){
            std::cout << "Not all packages can be upgraded. Wait for all the packages to become upgradable to the specified version " << version << ", or, run 'mcmodm ck_upd <version to update> <loader> <path to req.json>' to check that.\n";
        return;}
        std::cout << yellow<<"You are forcing this upgrade. ONLY the packages that can be upgraded will be upgraded. Run this in future to update more."<<reset_color<<std::endl;
        for (auto& [project_id, info] : packgs["installed"].items()) {
            if (project_id.starts_with("local:")){
                std::cout << "\033[33mPackages installed from a file will not be updated.\033[0m\n";
                continue;
            }
            if(!upgradeableMods.contains(project_id)){
                //std::cout<< yellow << "Package "<<cyan<<project_id<<yellow<<" Is not upgradible to version" << cyan<<version<<reset_color<<std::endl;
                continue;
            }
            remove_package(project_id, true);
            json ti;
            ti.push_back({{"version", version}, {"loader", json::array({info["loader"]})}}); // keep the same loader(s) as before
            install_mod(project_id, ti, true);
        }
    }
    for (auto& [project_id, info] : packgs["installed"].items()) {
        if (project_id.starts_with("local:")){
            std::cout << "\033[33mPackages installed from a file will not be updated.\033[0m\n";
        } else {
            bool up_to_date = false;
            /*std::cout << "Project ID: " << project_id << "\n";
            std::cout << "  File: " << info["file"] << "\n";    
            std::cout << "  Game version: " << info["game_version"] << "\n";
            std::cout << "  Loader: " << info["loader"] << "\n";*/
            if(info["game_version"] == version){
                std::cout << "Package " << project_id << " is already up to date for version " << version << ". Skipping.\n";
                up_to_date = true;
                continue;
            }
            if(!up_to_date){
                remove_package(project_id, true);
                json ti;
                ti.push_back({{"version", version}, {"loader", json::array({info["loader"]})}}); // keep the same loader(s) as before
                install_mod(project_id, ti, true);
                //continue;
            }
        }
    }
    std::string req_path = install_path;
    if (!req_path.empty() && req_path.back() != '/')
        req_path += '/';
    req_path += "req.json";
    json req_s;
    req_s.push_back({{"version", version}, {"loader", loaders}});
    std::ofstream ofs(req_path);
    if (!ofs.is_open()) {
        std::cerr << "Failed to write req.json to " << req_path << "\n";
        return;
    }
    ofs << req_s.dump(4);
    ofs.close();
    std::cout << "Updated req.json with new version " << version << ".\n";
}
std::vector<areUpdatable> pb::McModm::McModm::check_all_upgradeable(std::string& version, std::string& loader){
    //set_path(install_path);
    json packgs = load_packages();
    //size_t packgs_instal = packgs["installed"].size();
    std::vector<areUpdatable> updatables;

    for (auto& [project_id, info] : packgs["installed"].items()) {
        if(project_id.starts_with("local:")){std::cout<< "Package " << project_id << " Is a local package. Skipping.\n";continue;}
        bool upgradable = can_be_upgraded(project_id, version, info["loader"]);
        updatables.push_back({info["name"].get<std::string>(), project_id, upgradable});
        //std::cout << "Is " << info["name"].get<std::string>() <<" (" <<project_id<< ") upgradable to version " << version << "and loader" << info["loader"]<<": "<<(upgradable ? "\033[32mYes\033[0m" : "\033[31mNo\033[0m") << ".\n";
    }
    return updatables;
}
