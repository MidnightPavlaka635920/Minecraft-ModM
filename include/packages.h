#pragma once
#include <string>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

bool is_installed(const std::string& project_id);
void mark_installed(
    const std::string& project_id,
    const std::string& game_version,
    const std::string& loader,
    const std::string& file,
    const std::string& name
);
void set_path(const std::string& p);
json load_packages();
void save_packages(const json& j);