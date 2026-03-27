#pragma once
#include <string>
#include <vector>
struct search_result {
    std::string title;
    std::string author;
    std::string project_id;
};
std::vector<search_result> search_mods(const std::string& query);