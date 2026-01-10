#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
void remove_package(std::string package_id, std::string install_path, bool just_remove);