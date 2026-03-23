#ifdef _WIN32
#include <windows.h>
/*void enable_ansi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}*/
#endif
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "../include/curl_access.h"
#include <cstdio> // for FILE*, popen
#include <curl/curl.h>
#include "../include/install.h"
#include "../include/remove.h"
#include "../include/updateall.h"
#include "../include/list.h"
#include "../include/setup.h"
#include "../include/easy.h"
#include "../include/iff.h"
#include "../include/il.h"
#include "../include/ck_vers.h"
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
    << "  search <modname>                                  - List online mods matching <modname>\n"
    << "  install <modname> (options) <path>                - Install mod <modname> to <path>\n"
    << "  remove <modname> <path>                           - Remove mod <modname> from <path>\n"
    << "  updateall <version> <path>                        - Update all mods in <path> for game version <version>\n"
    << "  list <path>                                       - List installed mods in <path>\n"
    << "  setup <path> <version> <loader>                   - Setup req.json in <path> with specified version and loader\n"
    << "  easy_install <path>                               - Easy install mods from a list in <path>\n"
    << "  easy_remove <path>                                - Easy remove mods from a list in <path>\n"
    << "  iff <path-to-packages.json> <install_path>        - Install from a list of packages\n"
    << "  ck_upd <version> <loader> <path-to-req.json>      - Check if all packages can be upgraded\n"
    << "  il <file_to_install> <path_to_install> <name>     - Install a local file to the destination.\n"
    << "Version 1.5\n";
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: mcmodm <operation>\n";
        return 1;
    }

    std::string operation = argv[1];   // "ls" or "i"
    /*#ifdef _WIN32
        enable_ansi();
    #endif*/
    if (operation == "search") {
        if(argc < 3){
            std::cout << "Not enough arguments provided\n Usage: mcmodm search <modname>\n";
            return 1;
        }
        std::string pn = argv[2];          // mod name or slug
        curl_global_init(CURL_GLOBAL_DEFAULT);
        // Perform a search query on Modrinth
        std::string search_url = "https://api.modrinth.com/v2/search?query=" + url_encode(pn);
        std::string oData;

        try {
            oData = curl_to_string(search_url);
        } catch (const std::exception& e) {
            std::cerr << "Error fetching search results: " << e.what() << "\n";
            return 1;
        }

        json search_results = json::parse(oData);
        // LIST operation: display search results
        json hits = search_results["hits"];
        std::cout << "Search results for '" << pn << "':\n";
        std::cout << "Title - Slug - Project ID\n";
        for (const auto& hit : hits) {
            std::cout << hit["title"].get<std::string>() << " - "
                      << hit["slug"].get<std::string>() << " - "
                      << hit["project_id"].get<std::string>() << "\n";
        }

    } else if (operation == "install") {
        if (argc < 4) {
            std::cerr << "Usage: mcmodm install <modname> <path>\n";
            return 1;
        }
        std::string overwrite_loader, overwrite_version = "";
        /*for (int i = 4; i < argc; i++){
            std::string arg = argv[i];
            if (arg.rfind("--overwrite-loader=", 0) == 0){
                overwrite_loader = arg.substr(std::string("--overwrite-loader=").length());
            } else if(arg.rfind("--overwrite-version=", 0) == 0){
                overwrite_version = arg.substr(std::string("--overwrite-version=").length());
            }
        }*/
        std::vector<std::string> mods;  // project IDs
        std::string install_path;

        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];

            if (arg.rfind("--overwrite-loader=", 0) == 0) {
                overwrite_loader = arg.substr(19);
            }
            else if (arg.rfind("--overwrite-version=", 0) == 0) {
                overwrite_version = arg.substr(20);
            }
            else {
                mods.push_back(arg); // temporarily push everything else
            }
        }

        // Last element is always the install path
        if (mods.empty()) {
            std::cerr << "No install path provided\n";
            return 1;
        }

        install_path = mods.back();  // correct
        mods.pop_back();             // now mods only contains project IDs

        // sanity check
        if (mods.empty()) {
            std::cerr << "No mods provided to install\n";
            return 1;
        }
        //std::string install_path = argv[(argc - 1)]; // installation path
        
        // INSTALL operation: read requested version/loader from req.json
        std::string req_path = install_path;
        if (!req_path.empty() && req_path.back() != '/'){
            req_path += '/';
        }
        req_path += "req.json";

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
        auto loaders = overwrite_loader.empty()
            ? get_loaders(req[0]["loader"])
            : std::vector<std::string>{overwrite_loader};
        std::string version = overwrite_version.empty() ? req[0]["version"].get<std::string>() : overwrite_version;
        req = json::array({{{"version", version}, {"loader", loaders}}});
        for (int i = 2; i < (argc - 1); i++){
            std::string pn = argv[i];          // mod name or slug
            install_mod(pn, install_path, req, false);
        }
        //std::string pn = argv[2];          // mod name or slug
        //install_mod(pn, install_path, req, false);

        
    } else if (operation =="remove"){
        if (argc < 4) {
            std::cerr << "Usage: mcmodm remove <modname> <path>\n";
            return 1;
        }
        std::string install_path = argv[(argc - 1)]; // installation path
        for (int i = 2; i < (argc - 1); i++){
            std::string pn = argv[i];          // mod name or slug
            remove_package(pn, install_path, false);
        }
        //std::string pn = argv[2]; 
        //remove_package(pn, install_path, false);
    } else if (operation == "updateall"){
        if (argc < 4) {
            std::cerr << "Usage: mcmodm updateall <version> <path>\n";
            return 1;
        }
        std::string version = argv[2]; // game version
        std::string install_path = argv[3]; // installation path
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
        update_all_packages(version, install_path, loaders, req);
    } else if (operation == "list") {
        if (argc < 3) {
            std::cerr << "Usage: mcmodm list <path>\n";
            return 1;
        }
        std::string install_path = argv[2]; // installation path
        // Call the function to list installed packages
        list_packs(install_path);
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
        setup(path, version, loaders);
    } else if (operation == "help"){
        help();
    } else if(operation == "3"){
        std::cout << "OP 3! Actual project name: dricca" << std::endl;
    } else if(operation == "easy_install"){
        if (argc < 3) {
            std::cerr << "Usage: mcmodm easy_install <path>\n";
            return 1;
        }
        std::string install_path = argv[2]; // installation path
        easy_install(install_path);
    } else if(operation == "easy_remove"){
        if (argc < 3) {
            std::cerr << "Usage: mcmodm easy_remove <path>\n";
            return 1;
        }
        std::string install_path = argv[2]; // installation path
        easy_remove(install_path);
    } else if(operation == "iff"){
        if (argc < 4) {
            std::cerr << "Usage: mcmodm iff <path-to-packages.json> <install-path>\n";
            return 1;
        }
        std::string packages_path = argv[2]; // path to packages.json
        std::string install_path = argv[3]; // installation path
        iff(packages_path, install_path);

    }else if(operation == "ck_upd"){
        if (argc < 5) {
            std::cerr << "Usage: mcmodm ck_upd <version to update> <loader> <path to req.json>\n";
            return 1;
        }
        std::string version = argv[2];
        std::string loader = argv[3];
        std::string req_path = argv[4];
        //std::ifstream sdata(req_path);

        //std::ifstream sdat(reqjsonPath);
        /*if (!sdata.is_open()) {
            std::cerr << "Cannot open req.json\n";
            //return 1;
            throw std::runtime_error("Cannot open req.json");
        }*/
        //json req = json::parse (sdata);
        //check_update(version, loader, req);
        std::cout << "\033[33mWarning: If you have multiple loaders set up for plugins, you might want to run this command multiple times!\033[0m\n";
        check_all_upgradeable(version, loader, req_path);

    } else if (operation == "il"){
        if (argc < 6){
            std::cerr << "Usage: mcmodm il <file_to_install> <path_to_install> <name> <loader>\n";
            return 1;
        }
        std::string fti = argv[2]; // game version
        std::string install_path = argv[3]; // installation path
        std::string req_path = install_path;
        std::string name = "N/A";
        name = argv[4];
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
        std::string loader = argv[5];
        std::string version = req[0]["version"];
        install_local(install_path, fti, name, version, loader);
    
    }else if(operation == "listver"){
        if (argc < 3){
            std::cerr << "Usage: mcmodm listver <project_id>\n";
            return 1;
        }
        std::string project_id = argv[2];
        auto compatible_versions = list_compatible_versions(project_id);
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
    } else{
        std::cerr << "Unknown operation: " << operation << "\n WTF were you trying to do?\n Here goes little help:\n";
        help();
        return 1;
    }

    return 0;
    curl_global_cleanup();
}
