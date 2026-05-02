#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace pb::McModm{
void setup(std::string& path, std::string& version, std::vector<std::string>& loaders);
}
