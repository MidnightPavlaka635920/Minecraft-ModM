#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
// #include "../include/info.h"
#include "../include/mcmodm.h"
#include "../include/curl_access.h"
using json = nlohmann::json;
#include <vector>
std::vector<ModInfo> pb::McModm::McModm::mod_info(const std::string& query){
    std::vector<ModInfo> information;
    std::vector<std::string> get_authors;
    bool cf_mod = query.starts_with("cf:");
    std::string project_id = query;
    std::string url;
    url = "https://api.modrinth.com/v2/project/" + curl_utils::url_encode(query);
    if(cf_mod){
        project_id = query.substr(3);
        url = "https://api.curseforge.com/v1/mods/"+project_id;
    }
    json resp,response;
    std::string token;
    if(cf_mod){
        std::ifstream key(pb::McModm::McModm::getConfigDir()+"/api_key.txt");
        if(!key.is_open()){
            throw std::runtime_error("Could not open api_key.txt");
        }
        std::getline(key,token);

    }
    try {
        std::string ret;
        if(cf_mod){
            ret = curl_utils::curl_to_string_with_http_header(url,{"Accept: application/json","x-api-key: " + token},false);
        }else{
            ret = curl_utils::curl_to_string(url);
        }
        resp = json::parse(ret);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching mod information: " << e.what() << "\n";
    }
    if(cf_mod){response=resp["data"];}else{response=resp;}
    ModInfo info;
    info.name = response[(cf_mod?"name":"title")].get<std::string>();
    info.description = response[(cf_mod?"summary":"description")].get<std::string>();
    info.project_type = (cf_mod?std::to_string(response["classId"].get<int>()):response["project_type"].get<std::string>());
    info.project_id = (cf_mod?std::to_string(response["id"].get<int>()):response["id"].get<std::string>());
    if(!cf_mod){
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
    }
    if(cf_mod){
        for(auto author:response["authors"]){
            std::string an = author["name"];
            if(author.contains("name")){get_authors.push_back(an);}
        }
        info.authors = get_authors;
    }else{
        if (response.contains("author")) {
            //nfo.author = response["author"].get<std::string>();
            get_authors.push_back(response["author"].get<std::string>());
            info.authors = get_authors;
        } else if (response.contains("team")){
            std::string member_url = "https://api.modrinth.com/v2/project/" + query + "/members";
            try {
                std::string ret = curl_utils::curl_to_string(member_url);
                json member_response = json::parse(ret);
                for (const auto& member : member_response) {
                    get_authors.push_back(member["user"]["username"].get<std::string>());
                }
                info.authors = get_authors;
            } catch (const std::exception& e) {
                std::cerr << "Error fetching mod authors: " << e.what() << "\n";
            }
        }
    }

    information.push_back(info);
    return information;
}
