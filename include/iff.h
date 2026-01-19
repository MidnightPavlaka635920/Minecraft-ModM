#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
void iff(const std::string& packages_path, const std::string& install_path);