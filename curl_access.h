#pragma once
#include <string>

std::string curl_to_string(const std::string& url);
void curl_download_file(const std::string& url, const std::string& out_path);
std::string url_encode(const std::string& value);
