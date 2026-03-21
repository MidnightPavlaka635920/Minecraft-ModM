#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
void setup(std::string& path, std::string& version, std::vector<std::string>& loaders);