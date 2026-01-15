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
using json = nlohmann::json;


void remove_package(std::string package_id, std::string install_path, bool just_remove) {
    set_path(install_path);
    json j = load_packages();
    if (j["installed"].contains(package_id)) {
        std::cout << "Package found: " << package_id << "\n" << "Deleting the file" << "\n";

        std::string file_path = install_path; // + "/" + j["installed"][package_id]["file"].get<std::string>();
        if (!file_path.empty() && file_path.back() != '/'){
            file_path += '/';
        }
        file_path += j["installed"][package_id]["file"].get<std::string>();
        if (std::remove(file_path.c_str()) == 0) {
            std::cout << "Removed file successfully." << "\n";
        } else {
            std::perror(("Failed to remove file: " + file_path).c_str());
        }
        std::cout << "Updating package DB." << "\n";
        if(just_remove){return;}else{
        j["installed"].erase(package_id);
        save_packages(j);
        }
    } else {
        std::cout << "Package not found: " << package_id << "\n";
    }
}