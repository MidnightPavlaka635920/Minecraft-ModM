#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "../include/color.h"
// #include "../include/install.h"
#include "../include/mcmodm.h"
using json = nlohmann::json;
void pb::McModm::McModm::iff(const std::string& packages_path){    
    std::ifstream sdata(packages_path);
    if (!sdata.is_open()) {
        std::cerr << "Cannot open " << packages_path << "\n";
        return;
    }
    json packages = json::parse(sdata);
    size_t packgs_installed = packages["installed"].size();
    std::cout << "Packages to be installed: " << packgs_installed << "\n";
    unsigned int idx = 0;
    for (auto& [project_id, info] : packages["installed"].items()){
        std::cout << idx << ")" << info["name"].get<std::string>() << "\n";
        idx++;
    }
    std::cout << "Install these packages? (y/n, enter doesn't work): ";
    char install_confirm;
    std::cin >> install_confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (install_confirm != 'y' && install_confirm != 'Y'&& !install_path.empty()){return;}
    for (auto& [project_id, info] : packages["installed"].items()){
        if (project_id.starts_with("local:")){
            std::cout << yellow<<"Warning: "<<reset_color<<"Local package " << project_id << " is skipped.\n Use 'mcmodm il' to install it mmanually afterwards.\n" << cyan <<"File to install: " << reset_color << info["file"].get<std::string>() << "\n";
        } else{
            std::vector<std::string> loader_mod;
            loader_mod.push_back(info["loader"].get<std::string>());
            json req_mod = json::array({{"loader", loader_mod},{"version", info["version"].get<std::string>()}});
            install_mod(project_id, req_mod, false);
        }
    }
    std::cout << "Installation complete, unless everything was already installed.\n";
}
