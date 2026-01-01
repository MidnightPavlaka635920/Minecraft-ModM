#pragma once 
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
void list_packs (std::string& install_path);