#include <cstddef>
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
#include "../include/linenoise.hpp"
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
bool color;
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
    << "  mcmodm                                            - Start the interactive shell\n"
    << "  mcmodm <command> [args]                            - Run a command directly from the shell\n"
    << "\nInteractive shell:\n"
    << "  help                                              - Show this help\n"
    << "  exit                                              - Leave the shell\n"
    << "  Commands are entered without the 'mcmodm' prefix in the shell\n"
    << "  Example: mcmodm> list -i prod\n"
    << "\nAvailable commands:\n"
    << "  search <modname>                                       - List online mods matching <modname>\n"
    << "  install <modname>... [options] [path]                  - Install one or more mods to [path] (uses default if not specified)\n"
    << "     --overwrite-loader=<loader> --overwrite-version=<version> --vn=<version-number>\n"
    << "  remove <modname>... [path]                             - Remove one or more mods from [path] (uses default if not specified)\n"
    << "  updateall <version> [path] -f                          - Update all mods in [path] for game version <version> (uses default if not specified), -f is force\n"
    << "  list [path]                                            - List installed mods in [path] (uses default if not specified)\n"
    << "  setup <path> <version> <loader> (loader)               - Setup req.json in <path> with specified version and one or more loaders\n"
    << "  easy_install [path]                                    - Easy install mods from a list in [path] (uses default if not specified)\n"
    << "  easy_remove [path]                                     - Easy remove mods from a list in [path] (uses default if not specified)\n"
    << "  iff <path-to-packages.json> <install_path>             - Install from a list of packages\n" //72
    << "  ck_upd <version> path-to-req.json]                     - Check if all packages can be upgraded (uses default if not specified)\n"
    << "  il <file_to_install> <name> <loader> [path_to_install] - Install a local file to the destination (uses default if not specified)\n"
    << "  listver <project_id> [loader]                          - List compatible versions for a project with a specifiable loader\n"
    << "  info <project_id>                                      - Show detailed info for a project\n"
    << "  lscompl <version> <project_id>                         - Lists compatible loaders for specific version of the project\n"
    << "  setupinfo [path]                                       - Prints information about setup at path\n"
    << "  verify_installed [path]                                - Verify installed mods still exist on disk; optionally reinstall/remove missing ones\n"
    << "  listvernums <project_id> [game_version]                - Lists all version numbers for <project_id> with version [game_version] if specified\n"
    << "  instance <add|rm|ls>                                   - Adds, removes, or lists instances. -p/-i doesn't work\n"
    << "\tinstance add <instance_name> <instance_path>\n"
    << "\tinstance rm <instance_name>\n"
    << "\tinstance ls\n"
    << "\nGlobal path/instance selectors (available on most commands):\n"
    << "  -p <path>                                              - Use this path instead of the default one\n"
    << "  -i <instance_name>                                     - Use a saved instance instead of the default path\n"
    << "\nExamples:\n"
    << "  mcmodm\n"
    << "  mcmodm> help\n"
    << "  mcmodm> list -i my_server\n"
    << "  mcmodm install sodium -p /srv/minecraft/plugins\n"
    << "  mcmodm verify_installed -i my_server\n"
    << "  mcmodm instance add prod /srv/minecraft/plugins\n"
    << "Note: [path] arguments are optional if a default path is configured via config file.\n"
    << "Also, in most commands, you can specify path with -p <path> or -i <instance_name>\n"
    << "Version 2.2\n";
}
int runMcmodm(const std::vector<std::string>& arguments) {
    //if (arguments.size() < 2) {
    //    std::cerr << "Usage: mcmodm <operation>\n";
    //    return 1;
   // }
    //std::string operation = argv[1];   // "ls" or "i"
    std::string operation;
    std::string instance;
    std::string path;

    bool has_instance = false;
    bool has_path = false;

    std::vector<std::string> args;

    for (size_t i = 0; i < arguments.size(); ++i) {
        std::string arg = arguments[i];

        if (arg == "-i") {
            if (i + 1 >= arguments.size()) {
                std::cerr << "-i requires an instance\n";
                return 1;
            }
            if (has_path) {
                std::cerr << "-i and -p cannot be used together\n";
                return 1;
            }

            instance = arguments[++i];
            has_instance = true;
        }
        else if (arg == "-p") {
            if (i + 1 >= arguments.size()) {
                std::cerr << "-p requires a path\n";
                return 1;
            }
            if (has_instance) {
                std::cerr << "-i and -p cannot be used together\n";
                return 1;
            }

            path = arguments[++i];
            has_path = true;
        }
        else {
            args.push_back(arg);
        }
    }

    if (args.empty()) {
        std::cerr << "No operation specified\n";
        return 1;
    }
    if(!pb::McModm::McModm::getPath(path, instance).empty()){
        std::cout<<cyan<<"INFO: specifying path in the old way won't change anything. Use -p/-i if needed.\n"<<reset_color;
    }

    operation = args[0];

    try{
    if (operation == "search") {
        if(args.size() < 2){
            std::cout << "Not enough arguments provided\n Usage: mcmodm search <modname>\n";
            return 1;
        }

        std::string pn = args[1];          // mod name or slug
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
        if (args.size() < 2) {
            std::cerr << "Usage: mcmodm install <modname> [options] [path]\n";
            return 1;
        }
        std::string overwrite_loader, overwrite_version = "", version_number = "";
        std::vector<std::string> mods;  // project IDs
        std::string install_path;
        bool checkHash=true;
        for (size_t i = 1; i < args.size(); i++) {
            std::string arg = args[i];

            if (arg.rfind("--overwrite-loader=", 0) == 0) {
                overwrite_loader = arg.substr(19);
            }
            else if (arg.rfind("--overwrite-version=", 0) == 0) {
                overwrite_version = arg.substr(20);
            } else if(arg.rfind("--vn=",0) == 0){
                version_number = arg.substr(5);
            } else if(arg=="--noverify"){
                checkHash = false;
            }else {
                mods.push_back(arg); // temporarily push everything else
            }
        }

        if (mods.empty()) {
            std::cerr << "No arguments provided for install\n";
            return 1;
        }
        install_path = pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()){std::cerr<<red<<"No path vas provided!\n"<<reset_color;return 1;}
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
        //size_t modsamm = mods.size();
        //int index = 0;
        //for (const auto& mod:mods){
        //             // mod name or slug
        //    std::cout << green << "["<<std::to_string(index+1)<<"/"<<std::to_string(modsamm)<<"] "<<"Installing " << mod << "...\n"<<reset_color;
            InstallFlag tempIF = InstallFlag::None;
            tempIF = tempIF | (apm?InstallFlag::AutoPathManagement:InstallFlag::None);
            tempIF = tempIF | (checkHash?InstallFlag::VerifyHash:InstallFlag::None);
            modm.install_wrapper(mods, req,version_number,tempIF,InstallWrapper::NoCheckInstalled);
        //    ++index;
        //}
        //std::string pn = argv[2];          // mod name or slug
        //install_mod(pn, install_path, req, false);

        
    } else if (operation =="remove"){
        if (args.size() < 2) {
            std::cerr << "Usage: mcmodm remove <modname> [path]\n";
            return 1;
        }
        std::vector<std::string> mods;
        std::string install_path;
        for (size_t i = 1; i < args.size(); i++) {
            mods.push_back(args[i]);
        }
        if (mods.empty()) {
            std::cerr << "No mods provided\n";
            return 1;
        }
        install_path = pb::McModm::McModm::getPath(path, instance);
        pb::McModm::McModm modm(install_path + "/");
        std::ifstream sdata(install_path+"/req.json");
        json req = json::parse(sdata);
        bool apm = req[0].value("apm", false);
        for (size_t i = 0; i < mods.size(); i++){
            std::string pn = mods[i];          // mod name or slug
            std::cout << green << "["<<std::to_string(i)<<"/"<<std::to_string(mods.size())<<"] "<<"Removing " << pn << "...\n"<<reset_color;
            modm.remove_package(pn, false,apm);
        }
        //std::string pn = argv[2];
        //remove_package(pn, install_path, false);
    } else if (operation == "updateall"){
        if (args.size() < 2) {
            std::cerr << "Usage: mcmodm updateall <version> [path]\n";
            return 1;
        }
        std::string version = args[1]; // game version
        //bool force = false;
        std::string install_path;

        /*for (size_t i = 3; i < args.size(); i++) {
            std::string arg = args[i];

            if (arg == "-f" || arg == "--force") {
                force = true;
            }
        }*/

        // fallback to default path
        install_path = pb::McModm::McModm::getPath(path, instance);
        // still no path? error
        if (install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\n"
                      << "Usage: mcmodm updateall <version> [-p path] [--force]\n";
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
        modm.update_all_packages(version, loaders, req, args);
    } else if (operation == "list") {
        if (args.size() < 1) {
            std::cerr << "Usage: mcmodm list [ -p path]\n";
            return 1;
        }
        std::string install_path;
        install_path = pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()) {
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
        std::cout<<yellow<<"WARNING: Using -i/-p here will be ignored!"<<reset_color<<"\n";
        if (arguments.size() < 4){
            std::cerr << "Usage: mcmodm setup <path> <version> <loader> (loader)\nNote that -p/-i isn't available here. You must specify the path.";
            return 1;
        }
        std::vector <std::string> loaders;
        std::string path = arguments[1];
        std::string version = arguments[2];
        //std::string loader = argv[4];
        for (size_t i = 3; i < arguments.size(); i++) {
            loaders.push_back(arguments[i]);
        }
        pb::McModm::setup(path, version, loaders);
    } else if (operation == "help"){
        help();
    } else if(operation == "3"){
        std::cout << "OP 3! Actual project name: dricca" << std::endl;
        return 3;
    } else if(operation == "easy_install"){
        if (args.size() < 1) {
            std::cerr << "Usage: mcmodm easy_install [path]\n";
            return 1;
        }
        std::string install_path;
        install_path = pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm easy_install [path]\n";
            return 1;
        }
        //std::cout << install_path<<std::endl;
        pb::McModm::McModm modm(install_path);
        //install_path = argv[2]; // installation path
        modm.easy_install(color);
    } else if(operation == "easy_remove"){
        if (args.size() < 1) {
            std::cerr << "Usage: mcmodm easy_remove [path]\n";
            return 1;
        }
        std::string install_path;
        install_path = pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm easy_remove [path]\n";
            return 1;
        }
        pb::McModm::McModm modm(install_path + "/");
        //install_path = argv[2]; // installation path
        modm.easy_remove(color);
    } else if(operation == "iff"){
        if (args.size() < 3) {
            std::cerr << "Usage: mcmodm iff <path-to-packages.json> < -p <install-path> | -i <instance_name>>\n";
            return 1;
        }
        std::string packages_path = args[1]; // path to packages.json
        //std::string install_path = argv[3]; // installation path
        std::string install_path;
        install_path = pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()){
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm easy_remove [path]\n";
            return 1;
        }
        bool checkHash = true;
        if(std::find(args.begin(), args.end(),"--noverify")!=args.end())
            checkHash = false;
        pb::McModm::McModm modm(install_path + "/");
        InstallFlag cIF=(checkHash?InstallFlag::VerifyHash:InstallFlag::None);
        modm.iff(packages_path,cIF);

    }else if(operation == "ck_upd"){
        if (args.size() < 2) {
            std::cerr << "Usage: mcmodm ck_upd <version to update> [-p <install_path> | -i <instnce_name>]\n";
            return 1;
        }
        std::string version = args[1];
        //std::string loader = argv[3];
        std::string install_path;
        install_path = pb::McModm::McModm::getPath(path,instance);
        if(install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm ck_upd <version> <loader> [install path]\n";
            return 1;
        }
        pb::McModm::McModm modm(install_path);
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
        if (args.size() < 5) {
            std::cerr << "Usage: mcmodm il <file_to_install> <name> <loader> <type> [-p <path>|-i <insatnce>]\n";
            return 1;
        }
        std::string install_path;
        install_path = pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm il <file_to_install> <name> <loader> [path_to_install]\n";
            return 1;
        }
        std::string fti = args[1]; // game version
        //std::string install_path = argv[5]; // installation path
        std::string req_path = install_path;
        std::string name = "N/A";
        name = args[2];
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
        std::string loader = args[3];
        std::string type = args[4];
        std::string version = req[0]["version"];
        modm.install_local(fti, name, version, loader,type);

    }else if(operation == "listver"){
        if (args.size() < 2){
            std::cerr << "Usage: mcmodm listver <project_id>\n";
            return 1;
        }
        std::string project_id = args[1];
        std::string loader="";
        if(args.size()>2)
            loader = args[2];
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
        if (args.size() < 2){
            std::cerr << "Usage: mcmodm info <project_id>\n";
            return 1;
        }
        std::string project_id = args[1];
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
        if (args.size() < 3){
            std::cerr << "Usage: mcmodm lscompl <version> <project_id>\n";
            return 1;
        }
        std::string version = args[1];
        std::string project_id = args[2];
        auto compatible_loaders = pb::McModm::McModm::list_comp_loaders(version, project_id);
        std::cout <<cyan<< "Compatible loaders for project '" <<yellow<< project_id <<cyan<< "' and version '" <<yellow<< version << reset_color<<"':\n";
        for (const auto& loader : compatible_loaders) {
            std::cout << yellow<<" - "<<reset_color<< loader << "\n";
        }
    }else if(operation =="setupinfo"){
        if (args.size() < 1) {
            std::cerr << "Usage: mcmodm setupinfo [path]\n";
            return 1;
        }
        std::string install_path;
        install_path = pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmmodm setupinfo [path]\n";
            return 1;
        }
    pb::McModm::McModm modm(install_path);
    modm.getSetupInfo();
    } else if(operation=="listvernums"){
        if (args.size() <2)
            std::cerr << "Too few arguments.\nUsage: mcmodm listvernums <project_id> [game_version]\n";
        std::string game_version;
        std::string project_id = args[1];
        if(args.size() > 2){
            game_version = args[2];
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
    }else if(operation=="instance"){
        std::cout<<"WARNING: -p/-i will be ignored here!"<<"\n";
        if(args.size()<2){
            std::cout<<"Not enough arguments. Needs at least one more. Available options: \n\tmcmodm instance add <instance_name> <instance_path> - adds an instance\n\tmcmodm instance rm <instance_name> - removes specified instance\n\tmcmodm instance ls - lists instances\n";return 1;
        }
        if(args[1]=="add"){
            if(args.size()<4){
                std::cout<<"Too few arguments. Usage:\n\tmcmodm instance add <instance_name> <instance_path>\n";return 1;}
            pb::McModm::McModm::addInstance(args[3],args[2]);
            std::cout<<"Instance added successfully!\n";
        } else if(args[1]=="rm"){
            if(args.size()<3){
                std::cout<<"Too few arguments. Usage:\n\tmcomdm instance rm <instance name>\n";
                return 1;
            }
            pb::McModm::McModm::removeInstance(args[2]);
            std::cout<<green<<"Instance successfully removed!\n"<<reset_color;
        } else if(args[1]=="ls"){
            const auto& instances = pb::McModm::McModm::getInstances();
            std::cout<<green<<"name"<<cyan<<" - "<<green<<"path"<<reset_color<<"\n";
            for(auto&[name,insPath]:instances){
                std::cout<<yellow<<name<<cyan<<" - "<<yellow<<insPath<<reset_color<<"\n"; 
            }
        } else {
            std::cout<<"Unknown operation!\n";
            help();
            return 1;
        }
    }else if(operation=="test"){throw std::runtime_error("test");
    }else if(operation=="verify_installed" || operation=="verify-installed"){
        std::string install_path=pb::McModm::McModm::getPath(path, instance);
        if(install_path.empty()) {
            std::cerr << "No path provided. Either provide it in the command, or set up a default path.\nUsage: mcmodm verify_installed [path]\n";
            return 1;
        }
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
        bool apm = req[0].value("apm", false);
        InstallFlag cif = (apm?InstallFlag::AutoPathManagement:InstallFlag::None);
        modm.verify_all_mods(req,cif);
    }else if(operation=="get-deps"){
        if(args.size()<2){std::cout<<"Not enough arguments. Usage:\n\tmcmodm get-deps <project_id> [-p <path>/-i <instance>]\n";return 1;}
        auto install_path = pb::McModm::McModm::getPath(path,instance);
        pb::McModm::McModm modm(install_path);
        std::ifstream ifs(install_path+"/req.json");
        json req = json::parse(ifs);
        ifs.close();
        auto deps = modm.get_deps(args[1],"",req);
        std::cout<<yellow<<"Deps for this project:\n"<<reset_color;
        for(const auto& id:deps){
            std::cout<<id<<"\n";
        }
    }else{
        std::cerr << "Unknown operation: " << operation << "\n WTF were you trying to do?\nRun help for available commands.\n";
        //help();
        return 1;
    }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
    curl_global_cleanup();

}

std::vector<std::string> tokenize(const std::string& input)
{
    std::vector<std::string> tokens;
    std::string current;

    bool in_single_quote = false;
    bool in_double_quote = false;
    bool escaped = false;
    bool token_started = false;

    for (char c : input)
    {
        if (escaped)
        {
            current += c;
            escaped = false;
            token_started = true;
            continue;
        }

        if (c == '\\' && !in_single_quote)
        {
            escaped = true;
            token_started = true;
            continue;
        }

        if (c == '"' && !in_single_quote)
        {
            in_double_quote = !in_double_quote;
            token_started = true;
            continue;
        }

        if (c == '\'' && !in_double_quote)
        {
            in_single_quote = !in_single_quote;
            token_started = true;
            continue;
        }

        if (std::isspace(static_cast<unsigned char>(c))
            && !in_single_quote
            && !in_double_quote)
        {
            if (token_started)
            {
                tokens.push_back(current);
                current.clear();
                token_started = false;
            }

            continue;
        }

        current += c;
        token_started = true;
    }

    if (escaped)
        throw std::runtime_error("Trailing escape character");

    if (in_single_quote || in_double_quote)
        throw std::runtime_error("Unterminated quote");

    if (token_started)
        tokens.push_back(current);

    return tokens;
}

int runShell(){
    std::cout<< "\t\tThis is a McModm shell.\n\t\tFor help type help and press enter.\n";
    while(1){
        std::string input;
        std::string prompt = green+"mcmodm> "+reset_color;
        if(linenoise::Readline(prompt.c_str(),input)){std::cout<<"exit\n";break;}
        if(input.empty())
            continue;
        linenoise::AddHistory(input.c_str());
        auto args = tokenize(input);
        if(args[0]=="exit")
            break;
        int ret = runMcmodm(args);
        if(ret!=0)
            std::cout<<red<<"\n[exit status "<<ret<<"]\n"<<reset_color;
    }
    return 0;
}

int main(int argc, char* argv[]){
        #ifdef _WIN32
        if (!enable_ansi()) {
            
            std::cerr << "Warning: Failed to enable ANSI escape codes. Output may not be colored.\n";
            color = false;
            reset_color = "";
            yellow = "";
            cyan = "";
            red = "";
            green = "";
            color = false;
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

    if(argc<2)
        return runShell();
    std::vector<std::string> args(argv + 1, argv + argc);
    return runMcmodm(args);
}
