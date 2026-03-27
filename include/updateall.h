#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
struct areUpdatable{
    std::string name;
    std::string project_id;
    bool updatable;
};
void update_all_packages(std::string& version, std::string& install_path, std::vector<std::string>& loaders, json& req);
std::vector<areUpdatable> check_all_upgradeable(std::string& version, std::string& install_path, std::string& loader);