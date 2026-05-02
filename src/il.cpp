#include <string>
#include <nlohmann/json.hpp>
#include <filesystem>
//#include <stdio>
#include <fstream>
#include <iostream>
// #include "../include/packages.h"
#include "../include/mcmodm.h"
using json = nlohmann::json;
namespace fs = std::filesystem;
void pb::McModm::McModm::install_local(std::string& path_tif, std::string& name, std::string& version ,std::string& loader){
    std::string v_pn = "local:" + name;
    if (is_installed(v_pn)){
        std::cerr << "This file is already installed. If you want to reinstall it, please remove it first.\n";

    }
    //set_path(path_destination);
    std::cout << "Installing locally\n";
    std::cout << "\033[33mWarning:\033[0m Dependenices will NOT be automatically Installed!\n";
    std::cout << "Installing...\n";
    std::string dest = install_path + "/";
    try{
        fs::copy(path_tif, install_path);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Something went wrong and I don't know what. Hope this helps." << e.what();
    }
    fs::path p = (path_tif);
    std::string filename = p.filename().string();
    std::string pn = "local:" + filename;
    
    mark_installed(pn, version, loader, filename, name);
    //save_packages();
    std::cout << "Installed successfully\n";
}
