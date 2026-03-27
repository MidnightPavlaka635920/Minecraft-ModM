#pragma once 
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
using json = nlohmann::json;
struct list_info {
    std::string name;
    std::string project_id;
    std::string game_version;
};
std::vector<list_info> list_packs (std::string& install_path);