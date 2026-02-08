#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>
#include "../include/install.h"
#include "../include/remove.h"
#include "../include/curl_access.h"
#include "../include/packages.h"
#include <sstream>
#include <vector>
using json = nlohmann::json;
void easy_install(std::string& install_path){
    while (true){
        std::string query;
        std::cout << "Searching for (use q to quit, not just here): ";
        std::getline(std::cin, query);
        if (query == "q"){
            break;
        }
        std::string search_url = "https://api.modrinth.com/v2/search?query=" + url_encode(query);
        json search;
        try{
            search = json::parse(curl_to_string(search_url));
        } catch (...){
            std::cerr << "Something did not work. I don't know on which side the God is.\n" << std::endl;
            break;
        }
        json hits = search["hits"];
        if (hits.empty()){
            std::cout << "No results found for what you were searching. Did you type it in correctly? I can't help you. Quitting.\n" << std::endl;
            break;
        }
        std::cout << "This is what is found:\n";
        for (size_t h = 0;h < hits.size();h++){
            std::cout << h << ") " << hits[h]["title"].get<std::string>() << " - " << hits[h]["description"].get<std::string>() << " - " << hits[h]["project_id"].get<std::string>() << std::endl;
        }
        std::cout << "Which one to install? (type number in front of the item, a for all, or more seperated by commas without a space):";
        //unsigned int choice;
        //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string choice_line;
        std::getline(std::cin, choice_line);
        //std::cin >> choice;

        /*if (choice < 0 || choice >= (int)hits.size()) {
            std::cerr << "Invalid selection.\n";
            break;
        }*/
        std::vector<std::string> to_install;

        if (choice_line == "a" || choice_line == "A") {
            for (auto& hit : hits) {
                to_install.push_back(hit["project_id"]);
            }
        } else{
            std::stringstream ss(choice_line);
            std::string token;

            while (std::getline(ss, token, ',')) {
                try {
                    int idx = std::stoi(token);
                    if (idx < 0 || idx >= (int)hits.size()) {
                        std::cerr << "Invalid selection: " << idx <<" Use a number from the list, or a for all.\n";
                        continue;
                    }
                    to_install.push_back(hits[idx]["project_id"]);
                } catch (...) {
                    std::cerr << "Invalid input: " << token << "\n";
                }
            }
        }

//        std::string project_id = hits[choice]["project_id"];
        std::ifstream f(install_path + "/req.json");
        if (!f.is_open()) {
            std::cerr << "req.json not found in path.\n";
            continue;
        }
        json req = json::parse(f);
        for (const auto& project_id : to_install) {
            try {
                install_mod(project_id, install_path, req, false);
            } catch (const std::exception& e) {
                std::cerr << "Something went wrong during installation and here is what: " << e.what() << std::endl;
            }
        }
        std::cout << "Install another? (y/n): ";
        char again;
        std::cin >> again;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        //if(again == ""){break;}
        if (again != 'y' && again != 'Y'){break;}
    
    }
}

void easy_remove(std::string& install_path){
    while (true){
        set_path(install_path);
        json packgs = load_packages();
        auto& installed = packgs["installed"];
        std::vector<std::string> ids;
        //size_t packgs_instal = packgs["installed"].size();
        size_t idx = 0;
        std::cout << "Name - File - Game Version\n";
        for (auto& [project_id, info] : installed.items()) {
            std::cout << idx << ") " << info["name"].get<std::string>()
                    << " (" << info["file"].get<std::string>() << ") " << info["game_version"].get<std::string>() << "\n";
            ids.push_back(project_id);
            idx++;
        }
        std::cout << "\n";
        if (idx == 0) {
            std::cout << "No packages installed.\n";
            break;
        }
        std::cout << "Installed packages (" << idx << "):\n\n\n";
        std::cout << "What to remove? (type number in front of the item, a for all, or more seperated by commas without a space):";
        //unsigned int choice;
        //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string choice_line;
        std::getline(std::cin, choice_line);
        std::vector<std::string> to_remove;

        if (choice_line == "a" || choice_line == "A") {
            to_remove = ids;
        } else{
            std::stringstream ss(choice_line);
            std::string token;

            while (std::getline(ss, token, ',')) {
                try {
                    int i = std::stoi(token);
                    if (i < 0 || i >= (int)ids.size()) {
                        std::cerr << "Invalid selection: " << i << " Use a number from the list, or a for all.\n";
                        continue;
                    }
                    to_remove.push_back(ids[i]);
                } catch (...) {
                    std::cerr << "Invalid input: " << token << "\n";
                }
            }
        }
        if (to_remove.empty()) {
            std::cout << "Nothing selected. You should type at least one number, or a. Quitting.\n";
            break;
        }
        for (const auto& project_id : to_remove) {
            try {
                remove_package(project_id, install_path, false);
            } catch (const std::exception& e) {
                std::cerr << "Something went wrong during removal and here is what: " << e.what() << " I can't help it." << std::endl;
            }
        }
    }
}