#pragma once
#include <string>
#include <vector>
struct ModInfo {
    std::string name;
    std::vector<std::string> authors;
    std::string description;
    std::string project_type;
    std::string project_id;
};
std::vector<ModInfo> mod_info(const std::string& query);