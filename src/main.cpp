#ifdef _WIN32
#include <windows.h>
__attribute__((used))
const char* myFeelingsTowardsThisOS = "I hate Windows (11) and deving CLI on it is fucked up. If you are reading this with strings, there is more...";
bool enable_ansi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return false;
    if (!SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) return false;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    return true;
}
#endif
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <cstdio> // for FILE*, popen
#include <curl/curl.h>
// #include "../include/install.h"
// #include "../include/remove.h"
// #include "../include/updateall.h"
// #include "../include/list.h"
#include "../include/setup.h"
// #include "../include/easy.h"
// #include "../include/iff.h"
// #include "../include/il.h"
// #include "../include/ck_vers.h"
// #include "../include/search.h"
// #include "../include/info.h"
#include "../include/color.h"
#include "../include/mcmodm.h"
std::string reset_color;
std::string yellow;
std::string cyan;
std::string red;
std::string green;
using json = nlohmann::json;
std::vector<std::string> get_loaders(const json& j) {
    std::vector<std::string> loaders;

    if (j.is_string()) {
        loaders.push_back(j.get<std::string>());
    } else if (j.is_array()) {
        for (const auto& l : j) {
            loaders.push_back(l.get<std::string>());
        }
    } else {
        throw std::runtime_error("Invalid loader type (must be string or array)");
    }

    return loaders;
}
void help(){
    std::cout << "Available commands:\n"
    << "  search <modname>                                       - List online mods matching <modname>\n"
    << "  install <modname>... [options] [path]                  - Install one or more mods to [path] (uses default if not specified)\n"
    << "     --overwrite-loader=<loader> --overwrite-version=<version>\n"
    << "  remove <modname>... [path]                             - Remove one or more mods from [path] (uses default if not specified)\n"
    << "  updateall <version> [path] -f                          - Update all mods in [path] for game version <version> (uses default if not specified), -f is force\n"
    << "  list [path]                                            - List installed mods in [path] (uses default if not specified)\n"
    << "  setup <path> <version> <loader> (loader)               - Setup req.json in <path> with specified version and one or more loaders\n"
    << "  easy_install [path]                                    - Easy install mods from a list in [path] (uses default if not specified)\n"
    << "  easy_remove [path]                                     - Easy remove mods from a list in [path] (uses default if not specified)\n"
    << "  iff <path-to-packages.json> <install_path>             - Install from a list of packages\n"
    << "  ck_upd <version> path-to-req.json]                     - Check if all packages can be upgraded (uses default if not specified)\n"
    << "  il <file_to_install> <name> <loader> [path_to_install] - Install a local file to the destination (uses default if not specified)\n"
    << "  listver <project_id> [loader]                          - List compatible versions for a project with a specifiable loader\n"
    << "  info <project_id>                                      - Show detailed info for a project\n"
    << "  lscompl <version> <project_id>                         - Lists compatible loaders for specific version of the project\n"
    << "  setupinfo [path]                                       - Prints information about setup at path\n"
    << "  listvernums <project_id> [game_version]                - Lists all version numbers for <project_id> with version [game_version] if specified\n"
    << "Note: [path] arguments are optional if a default path is configured via config file.\n"
    << "Version 1.8\n";
}
// ...existing code...
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: mcmodm <operation>\n";
        return 1;
    }
    bool color = false;
    std::string operation = argv[1];   // "ls" or "i"
    std::string default_path = "";
    #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (!appdata) appdata = ".";
        std::string folder = std::string(appdata) + "\\mcmodm\\";
        //filesystem::create_directories(folder);
        std::string config_path = folder + "defpath.json";

    #else
        const char* home = std::getenv("HOME");
        if (!home) home = ".";
        std::string folder = std::string(home) + "/.config/mcmodm/";
        //std::filesystem::create_directories(folder);
        std::string config_path = folder + "defpath.json";
    #endif
    if (std::filesystem::exists(config_path)) {
        std::ifstream config_pt(config_path);
        if (config_pt.is_open()) {
            json config_json;
            config_json = json::parse(config_pt);
            if (config_json.contains("default_path")) {
                default_path = config_json["default_path"];
                //std::cout << "Default path loaded from config: " << default_path << "\n";
            } else {
                std::cerr << "Config file does not contain 'default_path'.\n";
            }
        }
    }
    #ifdef _WIN32
        if (!enable_ansi()) {
            
            std::cerr << "Warning: Failed to enable ANSI escape codes. Output may not be colored.\n";
            color = false;
            reset_color = "";
            yellow = "";
            cyan = "";
            red = "";
            green = "";
        } else {
            color = true;
        reset_color = "\033[0m";
        yellow = "\033[33m";
        cyan = "\033[36m";
        red = "\033[31m";
        green = "\033[32m";
        }

    #else
        color = true;
        reset_color = "\033[0m";
        yellow = "\033[33m";
        cyan = "\033[36m";
        red = "\033[31m";
        green = "\033[32m";
    #endif
    try{
    if (operation == "search") {
        if(argc < 3){
            std::cout << "Not enough arguments provided\n Usage: mcmodm search <modname>\n";
            return 1;
        }

        std::string pn = argv[2];          // mod name or slug
        auto results = pb::McModm::McModm::search_mods(pn);
        if (results.empty()) {
            std::cout << "No results found for query: " << pn << "\n";
        } else {
            std::cout << "Search results for query: " << pn << "\n";
            for (const auto& res : results) {
                std::cout <<yellow<< "Title: " << reset_color << res.title << yellow << ", Author: " << reset_color << res.author <<yellow<< ", Project ID: " << reset_color << res.project_id << "\n";
            }
        }

    } else if (operation == "install") {
        if (argc < 3) {
            std::cerr << "Usage: mcmodm install <modname> [options] [path]\n";
            return 1;
        }
        std::string overwrite_loader, overwrite_version = "", version_number = "";
        std::vector<std::string> mods;  // project IDs
        std::string install_path;

        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];

            if (arg.rfind("--overwrite-loader=", 0) == 0) {
                overwrite_loader = arg.substr(19);
            }
            else if (arg.rfind("--overwrite-version=", 0) == 0) {
                overwrite_version = arg.substr(20);
            } else if(arg.rfind("--vn=",0) == 0){
                version_number = arg.substr(5);
            }
            else {
                mods.push_back(arg); // temporarily push everything else
            }
        }

        if (mods.empty()) {
            std::cerr << "No arguments provided for install\n";
            return 1;
        }

        if (default_path.empty() || mods.size() > 1) {
            if (mods.size() < 2) {
                std::cerr << "No install path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm install <modname> [options] <path>\n";
                return 1;
            }
            install_path = mods.back();
            mods.pop_back();
        } else {
            install_path = default_path;
        }
        // for (const auto& mod : mods) {
            // std::cout << mod << "\n";
        // }
        // sanity check
        if (mods.empty()) {
            std::cerr << "No mods provided to install\n";
            return 1;
        }
        //std::string install_path = argv[(argc - 1)]; // installation path
        
        // INSTALL operation: read requested version/loader from req.json
        std::string req_path =  install_path;
        if (!req_path.empty() && req_path.back() != '/'){
            req_path += '/';
        }
        req_path += "req.json";
        pb::McModm::McModm modm(install_path + "/");
        std::ifstream sdata(req_path);

        //std::ifstream sdat(reqjsonPath);
        if (!sdata.is_open()) {
            std::cerr << "Cannot open req.json\n"<< "Run mcmodm setup <path> to create valid req.json if you didn't\n";
            //return 1;
            throw std::runtime_error("Cannot open req.json");
        }
        json req = json::parse (sdata);
        if (!req[0].contains("version") || !req[0].contains("loader")) {
            std::cerr << "req.json must contain 'version' and 'loader' fields.\n" << "Run mcmodm setup <path> to create valid req.json!\n";
            return 1;

        }
	/*std::vector<std::string> overwrite_loader_vector;
	std::vector<std::string> loader;
	if(!overwrite_loader.empty()){loader.push_back(overwrite_loader);} else{
	    for(const auto& l:req[0]["loader"]){loader.push_back(l.get<std::string>());}
	}*/
        bool apm = req[0].value("apm", false);
        auto loaders = overwrite_loader.empty()
            ? get_loaders(req[0]["loader"])
            : std::vector<std::string>{overwrite_loader};
        std::string version = overwrite_version.empty() ? req[0]["version"].get<std::string>() : overwrite_version;
        //req = json::array({{{"version", version}, {"loader", loaders}}});
        req[0]["version"] = version;
        req[0]["loaders"] = loaders;
        size_t modsamm = mods.size();
        int index = 0;
        for (const auto& mod:mods){
                     // mod name or slug
            std::cout << green << "["<<std::to_string(index+1)<<"/"<<std::to_string(modsamm)<<"] "<<"Installing " << mod << "...\n"<<reset_color;
            modm.install_mod(mod, req, apm,version_number,false);
            ++index;
        }
        //std::string pn = argv[2];          // mod name or slug
        //install_mod(pn, install_path, req, false);

        
    } else if (operation =="remove"){
        if (argc < 3) {
            std::cerr << "Usage: mcmodm remove <modname> [path]\n";
            return 1;
        }
        std::vector<std::string> mods;
        std::string install_path;
        for (int i = 2; i < argc; i++) {
            mods.push_back(argv[i]);
        }
        if (mods.empty()) {
            std::cerr << "No mods provided\n";
            return 1;
        }
        if (default_path.empty() || mods.size() > 1) {
            if (mods.size() < 2) {
                std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm remove <modname> [path]\n";
                return 1;
            }
            install_path = mods.back();
            mods.pop_back();
        } else {
            install_path = default_path;
        }
        pb::McModm::McModm modm(install_path + "/");
        std::ifstream sdata(install_path+"/req.json");
        json req = json::parse(sdata);
        bool apm = req[0].value("apm", false);
        for (size_t i = 0; i < mods.size(); i++){
            std::string pn = mods[i];          // mod name or slug
            std::cout << green << "["<<std::to_string(i)<<"/"<<std::to_string(mods.size())<<"] "<<"Remving " << pn << "...\n";
            modm.remove_package(pn, false,apm);
        }
        //std::string pn = argv[2];
        //remove_package(pn, install_path, false);
    } else if (operation == "updateall"){
        if (argc < 3) {
            std::cerr << "Usage: mcmodm updateall <version> [path]\n";
            return 1;
        }
        std::string version = argv[2]; // game version
        bool force = false;
        std::string install_path;

        for (int i = 3; i < argc; i++) {
            std::string arg = argv[i];

            if (arg == "-f" || arg == "--force") {
                force = true;
            } else if (install_path.empty()) {
                install_path = arg; // first non-flag becomes path
            }
        }

        // fallback to default path
        if (install_path.empty() && !default_path.empty()) {
            install_path = default_path;
        }

        // still no path? error
        if (install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\n"
                      << "Usage: mcmodm updateall <version> [path] [--force]\n";
            return 1;
        }
        pb::McModm::McModm modm(install_path + "/");
        std::string req_path = install_path;
        if (!req_path.empty() && req_path.back() != '/'){
            req_path += '/';
        }
        req_path += "req.json";

        std::ifstream sdata(req_path);

        //std::ifstream sdat(reqjsonPath);
        if (!sdata.is_open()) {
            std::cerr << "Cannot open req.json\n";
            //return 1;
            throw std::runtime_error("Cannot open req.json");
        }
        json req = json::parse (sdata);
        //std::string loader = req[0]["loader"];
        std::vector<std::string> loaders;
        for (const auto& l : req[0]["loader"]){
            loaders.push_back(l.get<std::string>());
        }
        modm.update_all_packages(version, loaders, req, force);
    } else if (operation == "list") {
        if (argc < 2) {
            std::cerr << "Usage: mcmodm list [path]\n";
            return 1;
        }
        std::string install_path;
        if (argc >= 3) {
            install_path = argv[2];
        } else if (!default_path.empty()) {
            install_path = default_path;
        } else {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm list [path]\n";
            return 1;
        }
            //install_path = argv[2]; // installation path
        //std::string install_path = argv[2]; // installation path
        // Call the function to list installed packages
        pb::McModm::McModm modm(install_path + "/");
        std::vector<list_info> installed_packages = modm.list_packs();
        if (installed_packages.empty()) {
            std::cout << "No packages installed. If that seems wrong, try again with a different path.\n";
        } else {
            std::cout << "Installed packages (" << installed_packages.size() << "):\n";
            std::cout << yellow<<"File Name"<<cyan<< " - "<<reset_color<< yellow<<"Project ID"<<cyan<< " - " <<reset_color<<yellow<<"Game Version\n" << reset_color;
            for (const auto& pkg : installed_packages) {
                std::cout << pkg.name << cyan << " - " << reset_color << pkg.project_id << cyan << " - " << reset_color << pkg.game_version << "\n";
            }
        }
    } else if (operation == "setup"){
        if (argc < 5){
            std::cerr << "Usage: mcmodm setup <path> <version> <loader> (loader)\n";
            return 1;
        }
        std::vector <std::string> loaders;
        std::string path = argv[2];
        std::string version = argv[3];
        //std::string loader = argv[4];
        for (int i = 4; i < argc; i++) {
            loaders.push_back(argv[i]);
        }
        pb::McModm::setup(path, version, loaders);
    } else if (operation == "help"){
        help();
    } else if(operation == "3"){
        std::cout << "OP 3! Actual project name: dricca" << std::endl;
        return 3;
    } else if(operation == "easy_install"){
        if (argc < 2) {
            std::cerr << "Usage: mcmodm easy_install [path]\n";
            return 1;
        }
        std::string install_path;
        if (argc >= 3) {
            install_path = argv[2];
        } else if (!default_path.empty()) {
            install_path = default_path;
        } else {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm easy_install [path]\n";
            return 1;
        }
        std::cout << install_path<<std::endl;
        pb::McModm::McModm modm(install_path);
        //install_path = argv[2]; // installation path
        modm.easy_install(color);
    } else if(operation == "easy_remove"){
        if (argc < 2) {
            std::cerr << "Usage: mcmodm easy_remove [path]\n";
            return 1;
        }
        std::string install_path;
        if (argc >= 3) {
            install_path = argv[2];
        } else if (!default_path.empty()) {
            install_path = default_path;
        } else {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm easy_remove [path]\n";
            return 1;
        }
        pb::McModm::McModm modm(install_path + "/");
        //install_path = argv[2]; // installation path
        modm.easy_remove(color);
    } else if(operation == "iff"){
        if (argc < 4) {
            std::cerr << "Usage: mcmodm iff <path-to-packages.json> <install-path>\n";
            return 1;
        }
        std::string packages_path = argv[2]; // path to packages.json
        //std::string install_path = argv[3]; // installation path
        std::string install_path;
        if (argc >= 3) {
            install_path = argv[2];
        } else if (!default_path.empty()) {
            install_path = default_path;
        } else {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm easy_remove [path]\n";
            return 1;
        }
        pb::McModm::McModm modm(install_path + "/");
        modm.iff(packages_path);

    }else if(operation == "ck_upd"){
        if (argc < 3) {
            std::cerr << "Usage: mcmodm ck_upd <version to update> [path to req.json]\n";
            return 1;
        }
        std::string version = argv[2];
        //std::string loader = argv[3];
        std::string req_path;
        if (argc >= 4) {
            req_path = argv[3];
        } else if (!default_path.empty()) {
            req_path = default_path;
        } else {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm ck_upd <version> <loader> [install path]\n";
            return 1;
        }
        pb::McModm::McModm modm(req_path);
        std::cout << yellow<<"Warning: If you have multiple loaders set up for plugins, you might want to run this command multiple times!\n" <<  reset_color;
        json mods = modm.load_packages();
        for(auto& [project_id, info]:mods["installed"].items()){
            if (project_id.starts_with("local:")){
                std::cout<<green<<"Skipping local package: " <<cyan<<project_id<<reset_color<<std::endl;
                continue;
            }
            bool updatable = modm.can_be_upgraded(project_id, version, info["loader"]);
            std::cout << "Is " << info["name"] <<" (" <<project_id<< ") upgradable to version " << version <<":"<<(updatable ? green + "Yes" + reset_color : red + "No" + reset_color) << ".\n";
        }

    } else if (operation == "il"){
        if (argc < 6) {
            std::cerr << "Usage: mcmodm il <file_to_install> <name> <loader> [path_to_install]\n";
            return 1;
        }
        std::string install_path;
        if (argc >= 6) {
            install_path = argv[5];
        } else if (!default_path.empty()) {
            install_path = default_path;
        } else {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm il <file_to_install> <name> <loader> [path_to_install]\n";
            return 1;
        }
        std::string fti = argv[2]; // game version
        //std::string install_path = argv[5]; // installation path
        std::string req_path = install_path;
        std::string name = "N/A";
        name = argv[3];
        if (!req_path.empty() && req_path.back() != '/'){
            req_path += '/';
        }
        req_path += "req.json";

        std::ifstream sdata(req_path);

        //std::ifstream sdat(reqjsonPath);
        if (!sdata.is_open()) {
            std::cerr << "Cannot open req.json\n";
            //return 1;
            throw std::runtime_error("Cannot open req.json");
        }
        pb::McModm::McModm modm(install_path + "/");
        json req = json::parse (sdata);
        //std::string loader = req[0]["loader"];
        std::string loader = argv[4];
        std::string type = argv[5];
        std::string version = req[0]["version"];
        modm.install_local(fti, name, version, loader,type);

    }else if(operation == "listver"){
        if (argc < 3){
            std::cerr << "Usage: mcmodm listver <project_id>\n";
            return 1;
        }
        std::string project_id = argv[2];
        std::string loader="";
        if(argc>3)
            loader = argv[3];
        auto compatible_versions = pb::McModm::McModm::list_compatible_versions(project_id,loader);
        std::cout << "Compatible versions for project '" << project_id << "':\n";
        int per_line = 3;

        for (size_t i = 0; i < compatible_versions.size(); i++) {
            std::cout << compatible_versions[i];
            if ((i + 1) % per_line != 0 && i != compatible_versions.size() - 1) {
                std::cout << " - ";
            }
            if ((i + 1) % per_line == 0) {
                std::cout << "\n";
            }
        }

        // final newline if needed
        if (compatible_versions.size() % per_line != 0) {
            std::cout << "\n";
        }
        std::cout << std::endl;
    } else if(operation == "info"){
        if (argc < 3){
            std::cerr << "Usage: mcmodm info <project_id>\n";
            return 1;
        }
        std::string project_id = argv[2];
        auto info = pb::McModm::McModm::mod_info(project_id);
        if (info.empty()) {
            std::cout << "No information found for project ID: " << project_id << "\n";
        } else {
            const auto& mod = info[0];
            std::cout << yellow<<"Name: "<<reset_color << mod.name << cyan<<" Project ID: " <<reset_color<< mod.project_id << "\n";
            std::cout << yellow<<"Description: "<<reset_color << mod.description << "\n";
            std::cout << cyan<< "Project Type: " <<reset_color<< mod.project_type << "\n";
            std::cout << yellow<<"Authors: "<<reset_color;
            for (size_t i = 0; i < mod.authors.size(); i++) {
                std::cout << mod.authors[i];
                if (i != mod.authors.size() - 1) {
                    std::cout << cyan<<", "<<reset_color;
                }
            }
            std::cout << "\n";
            std::cout << yellow<<"Client Side: "<<reset_color << (mod.cs ? green + "Yes" + reset_color : red + "No" + reset_color) << "\n";
            std::cout << yellow<<"Server Side: "<<reset_color << (mod.ss ? green + "Yes" + reset_color : red + "No" + reset_color) << "\n";
        }

    } else if(operation == "lscompl"){
        if (argc < 4){
            std::cerr << "Usage: mcmodm lscompl <version> <project_id>\n";
            return 1;
        }
        std::string version = argv[2];
        std::string project_id = argv[3];
        auto compatible_loaders = pb::McModm::McModm::list_comp_loaders(version, project_id);
        std::cout <<cyan<< "Compatible loaders for project '" <<yellow<< project_id <<cyan<< "' and version '" <<yellow<< version << reset_color<<"':\n";
        for (const auto& loader : compatible_loaders) {
            std::cout << yellow<<" - "<<reset_color<< loader << "\n";
        }
    }else if(operation =="setupinfo"){
        if (argc < 2) {
            std::cerr << "Usage: mcmodm setupinfo [path]\n";
            return 1;
        }
        std::string install_path;
        if (argc >= 3) {
            install_path = argv[2];
        } else if (!default_path.empty()) {
            install_path = default_path;
        } else {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmmodm setupinfo [path]\n";
            return 1;
        }
    pb::McModm::McModm modm(install_path);
    modm.getSetupInfo();
    } else if(operation=="listvernums"){
        if (argc <3)
            std::cerr << "Too few arguments.\nUsage: mcmodm listvernums <project_id> [game_version]\n";
        std::string game_version;
        std::string project_id = argv[2];
        if(argc>=4){
            game_version = argv[3];
        }
        std::vector<version_names_info> version_numbers = pb::McModm::McModm::list_version_nums(project_id, game_version);
        std::cout<< yellow<<"Version Number" << cyan<<" - "<<yellow<<"Game Versions"<<reset_color<<"\n";
        for(const auto& version_num:version_numbers){
            std::cout << version_num.ver_info<<cyan<<" -"<<reset_color;
            for(auto& game_ver_cur:version_num.game_ver){
                std::cout<< " "<<game_ver_cur;
            }
            std::cout<<"\n";
        }
    }else {
        std::cerr << "Unknown operation: " << operation << "\n WTF were you trying to do?\n Here goes little help:\n";
        help();
        return 1;
    }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
    curl_global_cleanup();

}
