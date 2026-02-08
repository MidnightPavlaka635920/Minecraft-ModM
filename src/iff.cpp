#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "../include/install.h"
using json = nlohmann::json;
void iff(const std::string& packages_path, const std::string& install_path){    
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
    std::string req_path = install_path;
    if (!req_path.empty() && req_path.back() != '/'){
        req_path += '/';
    }
    req_path += "req.json";

    std::ifstream req_json(req_path);

    if (!req_json.is_open()) {
        std::cerr << "Cannot open req.json\n";
        //return 1;
        throw std::runtime_error("Cannot open req.json");
    }
    json req = json::parse(req_json);
    char install_confirm;
    std::cin >> install_confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (auto& [project_id, info] : packages["installed"].items()){
        install_mod(project_id, install_path, req, false);
    }
    std::cout << "Installation complete, unless everything was already installed.\n";
    if (install_confirm != 'y' && install_confirm != 'Y'){return;}
}