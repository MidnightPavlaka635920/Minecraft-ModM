#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include "../include/mcmodm.h"
using json = nlohmann::json;
void pb::McModm::setup(std::string& path, std::string& version, std::vector<std::string>& loaders){
    std::string req_path = path;
    if (!req_path.empty() && req_path.back() != '/')
        req_path += '/';
    req_path += "req.json";
    json req_s;
    std::cout << "Should this be auto-managed path (you should enter minecraft path for this)?\nIt automatically downloads right type to right folder.(Y/n): ";
    std::string apm_choice;
    bool useAPM = false;
    std::getline(std::cin, apm_choice);
    if(apm_choice == "Y" || apm_choice == "y" or apm_choice==""){
        useAPM = true;
        std::cout << "Y";
    }
    req_s.push_back({{"version", version}, {"loader",loaders},{"apm",useAPM}});
    std::ofstream ofs(req_path);
    if (!ofs.is_open()) {
        std::cerr << "Failed to write req.json to " << req_path << "\n";
        return;
    }
    ofs << req_s.dump(4);
    ofs.close();
    std::cout <<"Setup created successfully at " << req_path << ". Remember that!\n";
    std::cout << "Do you want to save the path to a file in order not to type it every time you run the program? (Y/n): ";
    std::string save_choice;
    std::getline(std::cin, save_choice);
    if (save_choice == "y" || save_choice == "Y" || save_choice == "") {
        #ifdef _WIN32
            const char* appdata = std::getenv("APPDATA");
            if (!appdata) appdata = ".";
            std::string folder = std::string(appdata) + "\\mcmodm\\";
            std::filesystem::create_directories(folder);
            std::string config_path = folder + "defpath.json";
        #else
            const char* home = std::getenv("HOME");
            if (!home) home = ".";
            std::string folder = std::string(home) + "/.config/mcmodm/";
            std::filesystem::create_directories(folder);
            std::string config_path = folder + "defpath.json";
        #endif
        std::filesystem::path p(path);
        std::string path_abs = std::filesystem::absolute(p).string();
        std::ofstream config_ofs(config_path);
        if (!config_ofs.is_open()) {
            std::cerr << "Failed to write config file to " << config_path << "\n";
            return;
        }
        config_ofs << json{{"default_path", path_abs}}.dump(4);
        std::cout << "Default path saved successfully\n";
        config_ofs.close();
    }
}
