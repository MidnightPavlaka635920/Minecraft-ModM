#pragma once
#include <string>
#include <vector>
struct ModInfo {
    std::string name;
    std::vector<std::string> authors;
    std::string description;
    std::string project_type;
    std::string project_id;
    bool ss = false;
    bool cs = false;
};
std::vector<ModInfo> mod_info(const std::string& query);