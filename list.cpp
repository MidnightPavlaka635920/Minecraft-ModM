#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "../include/list.h"
#include "../include/packages.h"
using json = nlohmann::json;

void list_packs (std::string& install_path){
    set_path(install_path);
    json packgs = load_packages();
    size_t packgs_instal = packgs["installed"].size();
    if (packgs_instal == 0) {
        std::cout << "No packages installed.\n";
        return;
    }
    std::cout << "Installed packages (" << packgs_instal << "):\n";
    std::cout << "File Name - Project ID - Game Version\n";
    for (auto& [project_id, info] : packgs["installed"].items()) {
        std::cout << info["file"] << " - " << project_id << " - " << info["game_version"] << "\n";
    }
}