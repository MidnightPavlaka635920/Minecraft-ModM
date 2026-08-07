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
struct upgradeableModInfo{
    std::string project_id;
    json info;
};
#include <unordered_set>
void pb::McModm::McModm::update_all_packages(std::string& version, std::vector<std::string>& loaders, json& req, bool force) {
    if (req[0]["version"] == version&&!force) {
        std::cout << "No version change detected (" << version << "). Skipping update.\n";
        return;
    }
    const bool apm = req[0].value("apm", false);
    //set_path(install_path);
    json packgs = load_packages();
    size_t packgs_instal = packgs["installed"].size();
    std::cout << "Found " << packgs_instal << " packages to upgrade.\n";
    //std::cout << packgs["installed"].dump(4) << "\n";
    bool all_updatable = true;
    //std::vector<bool> upgradibleList;
    std::unordered_set<std::string> upgradeableMods;
    std::vector<upgradeableModInfo> plan;
    for (auto& [project_id, info] : packgs["installed"].items()) {
        bool upgradable = false;
        if (project_id.starts_with("local:")){
            std::cout << "\033[33mPackages installed from a file will not be updated.\033[0m\n";
            continue;
        } else{
            upgradable = can_be_upgraded(project_id, version, info["loader"]);
            plan.push_back({project_id, info});
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
        for (auto& mod : plan) {
            int index = 0;
            if(!upgradeableMods.contains(mod.project_id)){
                //std::cout<< yellow << "Package "<<cyan<<project_id<<yellow<<" Is not upgradible to version" << cyan<<version<<reset_color<<std::endl;
                continue;
            }
            remove_package(mod.project_id, true,apm);
            json ti;
            ti.push_back({{"version", version}, {"loader", json::array({mod.info["loader"]})}}); // keep the same loader(s) as before
            std::cout << green << "["<<std::to_string(index)<<"/"<<std::to_string(plan.size())<<"] "<<"Installing " << mod.project_id << "...\n";
            install_mod(mod.project_id, ti, apm, (const std::string)"", true);
            index++;
        }
    }
    int index = 0;
    for (auto& mod:plan) {
            bool up_to_date = false;
            if(mod.info["game_version"] == version){
                std::cout << "Package " << mod.project_id << " is already up to date for version " << version << ". Skipping.\n";
                up_to_date = true;
                continue;
            }
            if(!up_to_date){
                remove_package(mod.project_id, true,apm);
                json ti;
                ti.push_back({{"version", version}, {"loader", json::array({mod.info["loader"]})}}); // keep the same loader(s) as before
                std::cout << green << "["<<std::to_string(index)<<"/"<<std::to_string(plan.size())<<"] "<<"Installing " << mod.project_id << "...\n";
                install_mod(mod.project_id, ti, apm, (const std::string)"",true);
                //continue;
                index++;
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
