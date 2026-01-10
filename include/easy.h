#pragma once 
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
using json = nlohmann::json;
void easy_install(std::string& install_path);
void easy_remove(std::string& install_path);