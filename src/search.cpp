#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "../include/search.h"
#include "../include/curl_access.h"
#include <curl/curl.h>
#include <iostream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::vector<search_result> search_mods(const std::string& query) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    // Perform a search query on Modrinth
    std::string search_url = "https://api.modrinth.com/v2/search?query=" + url_encode(query);
    std::string oData;

    try {
        oData = curl_to_string(search_url);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching search results: " << e.what() << "\n";
        throw std::runtime_error("Error fetching search results");
    }
    std::vector<search_result> results;

    json search_results = json::parse(oData);
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