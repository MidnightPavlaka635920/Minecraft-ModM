#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
void setup(std::string& path, std::string& version, std::string& loader);