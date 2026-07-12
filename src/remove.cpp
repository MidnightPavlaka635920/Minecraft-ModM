#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <cstdio> // for FILE*, popen
//#include <curl/curl.h>
//#include "../include/packages.h"
#include "../include/mcmodm.h"
using json = nlohmann::json;


void pb::McModm::McModm::remove_package(std::string package_id, bool just_remove) {
    //set_path(install_path);
    json j = load_packages();
    if (j["installed"].contains(package_id)) {
        std::cout << "Package found: " << package_id << "\n" << "Deleting the file" << "\n";
        ProjectType type = getProjectType(j["installed"][package_id]["type"].get<std::string>()); 
        std::string file_path = install_path; // + "/" + j["installed"][package_id]["file"].get<std::string>();
        if (!file_path.empty() && file_path.back() != '/'){
            file_path += '/';
        }
        file_path += getInstallDirectory(type).string() + "/";
        file_path += j["installed"][package_id]["file"].get<std::string>();
        if (std::remove(file_path.c_str()) == 0) {
            std::cout << "Removed file successfully." << "\n";
        } else {
            std::perror(("Failed to remove file: " + file_path).c_str());
        }
        if(just_remove){return;}else{
        j["installed"].erase(package_id);
        save_packages(j);
        }
    } else {
        std::cout << "Package not found: " << package_id << "\n";
    }
}
