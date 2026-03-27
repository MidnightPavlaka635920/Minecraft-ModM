#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "../include/list.h"
#include "../include/packages.h"
using json = nlohmann::json;

std::vector<list_info> list_packs (std::string& install_path){
    set_path(install_path);
    json packgs = load_packages();
    size_t packgs_instal = packgs["installed"].size();
    std::vector<list_info> installed_packgs;
    if (packgs_instal == 0) {
        std::cout << "No packages installed. If that seems wrong, try again with a different path.\n";
        return installed_packgs;
    }
    //std::cout << "Installed packages (" << packgs_instal << "):\n";
    //std::cout << "File Name - Project ID - Game Version\n";
    for (auto& [project_id, info] : packgs["installed"].items()) {
        //std::cout << info["name"].get<std::string>() << " - " << project_id << " - " << info["game_version"].get<std::string>() << "\n";
        list_info li;
        li.name = info["name"].get<std::string>();
        li.project_id = project_id;
        li.game_version = info["game_version"].get<std::string>();
        installed_packgs.push_back(li);
    }
    return installed_packgs;
}