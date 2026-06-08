#pragma once
#include <string>
#include <vector>
namespace pb::curl_utils{
    std::string curl_to_string(const std::string& url, bool doProgressAnimation = false);
    void curl_download_file(const std::string& url, const std::string& out_path, bool doProgressAnimation = true);
    std::string url_encode(const std::string& value);
    std::string curl_to_string_with_http_header(std::string url, std::vector<std::string> headersVec, bool doProgressAnimation);
}
__attribute__((used))
const char a_thing[] = "In case you are wondering who is in 450479388_496240406415295_5860082617347840614_n.jpg, take a look at the source of my last project. It would be clear ;)";
