#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
using json = nlohmann::json;
void setup(std::string& path, std::string& version, std::vector<std::string>& loaders) {
    std::string req_path = path;
    if (!req_path.empty() && req_path.back() != '/')
        req_path += '/';
    req_path += "req.json";
    json req_s;
    req_s.push_back({{"version", version}, {"loader",loaders}});
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
        std::filesystem::path p(req_path);
        std::string path_abs = std::filesystem::absolute(p).string();
        std::ofstream config_ofs(config_path);
        if (!config_ofs.is_open()) {
            std::cerr << "Failed to write config file to " << config_path << "\n";
            return;
        }
        config_ofs << json{{"default_path", path_abs}}.dump(4);
        config_ofs.close();
    }
}