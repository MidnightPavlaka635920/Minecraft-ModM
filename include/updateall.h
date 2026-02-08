#pragma once
#include <string>
#include <nlohmann/json.hpp>
void update_all_packages(std::string& version, std::string& install_path, std::string& loader, json& req);
void check_all_upgradeable(std::string& version, std::string& install_path, std::string& loader);