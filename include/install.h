#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "curl_access.h"
using json = nlohmann::json;

void install_mod(const std::string& pn, const std::string& install_path, const json& req, bool just_install);