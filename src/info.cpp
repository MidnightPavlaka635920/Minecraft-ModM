#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
// #include "../include/info.h"
#include "../include/mcmodm.h"
#include "../include/curl_access.h"
using json = nlohmann::json;
#include <vector>
std::vector<ModInfo> McModm::mod_info(const std::string& query){
    std::vector<ModInfo> information;
    std::string url = "https://api.modrinth.com/v2/project/" + query;
    json response;
    try {
        std::string ret = curl_to_string(url);
        response = json::parse(ret);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching mod information: " << e.what() << "\n";
    }
    ModInfo info;
    info.name = response["title"].get<std::string>();
    info.description = response["description"].get<std::string>();
    info.project_type = response["project_type"].get<std::string>();
    info.project_id = response["id"].get<std::string>();
    if (response["client_side"] == "required"){
        info.cs = true;
    } else if (response["client_side"] == "unsupported"){
        info.cs = false;
    }
    if (response["server_side"] == "required"){
        info.ss = true;
    } else if (response["server_side"] == "unsupported"){
        info.ss = false;
    }
    std::vector<std::string> get_authors;
    if (response.contains("author")) {
        //info.author = response["author"].get<std::string>();
        get_authors.push_back(response["author"].get<std::string>());
        info.authors = get_authors;
    } else if (response.contains("team")){
        std::string member_url = "https://api.modrinth.com/v2/project/" + query + "/members";
        try {
            std::string ret = curl_to_string(member_url);
            json member_response = json::parse(ret);
            for (const auto& member : member_response) {
                get_authors.push_back(member["user"]["username"].get<std::string>());
            }
            info.authors = get_authors;
        } catch (const std::exception& e) {
            std::cerr << "Error fetching mod authors: " << e.what() << "\n";
        }
    }

    information.push_back(info);
    return information;
}