#include <vector>
#include <string>
#include <nlohmann/json.hpp>
//#include "../include/search.h"
#include "../include/mcmodm.h"
#include "../include/curl_access.h"
#include <curl/curl.h>
#include <iostream>
#include<fstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::vector<search_result> pb::McModm::McModm::search_mods(const std::string& query) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    // Perform a search query on Modrinth
    std::string search_url = "https://api.modrinth.com/v2/search?query=" + curl_utils::url_encode(query);
    std::string resultData;

    try {
        resultData = curl_utils::curl_to_string(search_url);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching search results: " << e.what() << "\n";
        throw std::runtime_error("Error fetching search results");
    }
    std::vector<search_result> results;

    json search_results = json::parse(resultData);
    // LIST operation: display search results
    json hits = search_results["hits"];
    for (const auto& hit : hits) {
        results.push_back({
            hit["title"].get<std::string>(),
            hit["author"].get<std::string>(),
            hit["project_id"].get<std::string>()
        });
    }
    return results;
}
std::vector<search_result> pb::McModm::McModm::search_mods_cf(std::string& query){
    const char* home= std::getenv("HOME");
    std::string key_path = std::string(home)+"/.config/mcmodm/";
    std::ifstream key(key_path+"/api_key.txt");
    if(!key.is_open()){
        throw std::runtime_error("Could not open api_key.txt");
    }
    std::string token;
    std::getline(key,token);
    std::string url = "https://api.curseforge.com/v1/mods/search?gameId=432&searchFilter=" + pb::curl_utils::url_encode(query) + "&sortOrder=desc&pageSize=10&index=0";
    std::string res = pb::curl_utils::curl_to_string_with_http_header(url,{"Accept: application/json","x-api-key: " + token},false);
    json sr = json::parse(res);
    std::vector<search_result>results;
    for(const auto& project:sr.at("data")){
    std::string author="Unknown";
    if (project.contains("authors") &&
        project["authors"].is_array() &&
        !project["authors"].empty()) {
        author = project["authors"][0].value("name", "Unknown");
    }

        results.push_back({
            project.value("name","Unknown"),//get<std::string>(),
            author,
            std::to_string(project.value("id",0))
        });
    }
    return results;
}
