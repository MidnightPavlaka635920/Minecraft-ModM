#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>
//#include "../include/install.h"
//#include "../include/remove.h"
//#include "../include/curl_access.h"
//#include "../include/packages.h"
//#include "../include/search.h"
// #include "../include/list.h"
#include <sstream>
#include <vector>
// #include "../include/color.h"
#include "../include/mcmodm.h"
#include "../include/color.h"
using json = nlohmann::json;
using string = std::string;
void pb::McModm::McModm::easy_install(bool color){
    while (true){
        std::string query;
        std::cout << "Searching for: ";
        std::getline(std::cin, query);
        std::vector<search_result> hits = search_mods(query);
        size_t hits_index =0;
        for (auto& result: hits){
            std::cout << cyan << hits_index << reset_color << " - " << yellow << "Title: " << reset_color << result.title << yellow << ", Author: " << reset_color << result.author << yellow << ", Project ID: " << reset_color << result.project_id << "\n";
            hits_index++;
        }
        if (hits.empty()) {
            std::cout << "No results found for query: " << query << "\n";
            break;
        }
        std::cout << "Which one to install? (type number in front of the item, a for all, or more seperated by commas without a space), q to quit: ";
        std::string choice_line;
        std::getline(std::cin, choice_line);
        if (choice_line == "q"){
            break;
        }
        
        std::vector<std::string> to_install;
        if (choice_line == "a" || choice_line == "A") {
            for (auto& hit : hits) {
                to_install.push_back(hit.project_id);
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
                    to_install.push_back(hits[idx].project_id);
                } catch (...) {
                    std::cerr << "Invalid input: " << token << "\n";
                }
            }
        }

//        std::string project_id = hits[choice]["project_id"];
        std::filesystem::path req_path(install_path + "/req.json");
        std::cout << req_path<<std::endl;
        std::ifstream f(req_path);
        if (!f.is_open()) {
            std::cerr << "req.json not found in path.\n";
            continue;
        }
        json req = json::parse(f);
        for (const auto& project_id : to_install) {
            try {
                install_mod(project_id, req, false);
            } catch (const std::exception& e) {
                std::cerr << "Something went wrong during installation and here is what: " << e.what() << std::endl;
            }
        }
        std::cout << "Install another? (y/n): ";
        std::string again;
        std::getline(std::cin, again);
         if (again.empty() || again == "Y" || again == "y") {
             continue;
        } else {
            break;
        }   
    }
}

void pb::McModm::McModm::easy_remove(bool color){
    while (true){
        std::vector<list_info> installed_packages = list_packs();
        std::vector<std::string> ids;
        size_t idx = 0;
        for (const auto& pkg : installed_packages) {
            std::cout << yellow << idx << reset_color << " - " << cyan << "Name: " << reset_color << pkg.name << cyan << ", Project ID: " << reset_color << pkg.project_id << cyan << ", Installed version: " << reset_color << pkg.game_version << "\n";
            ids.push_back(pkg.project_id);
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
                remove_package(project_id, false);
            } catch (const std::exception& e) {
                std::cerr << "Something went wrong during removal and here is what: " << e.what() << " I can't help it." << std::endl;
            }
        }
    }
}
