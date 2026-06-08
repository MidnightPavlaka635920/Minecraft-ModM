#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#endif
#include <vector>
#include <string>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <curl/curl.h>
#include <stdexcept>
#include <iostream>
#include "../include/curl_access.h"
size_t progress_print(void* ptr, curl_off_t total, curl_off_t now, curl_off_t, curl_off_t){
    if (total >0){
        float progress = (float)now / total;
        int ppo = progress * 100;
        int progress_old = 0.0;
        int barWidth = 50;
        int fill = barWidth * progress;
        if (ppo != progress_old){
            std::cout << "\r[";
            for (int i = 0; i < barWidth; ++i) {
                std::cout << (i < fill ? "#" : "-");
            }
            std::cout << "] " << int(progress * 100.0) << "%";
        }
        progress_old = ppo;
    } else {
        std::cout << "\rDownloading... " << now << " bytes";
    }
    return 0;
}
static size_t curl_write_cb(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t total = size * nmemb;
    std::string* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(contents), total);
    return total;
}

std::string pb::curl_utils::curl_to_string(const std::string& url, bool doProgressAnimation)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl_easy_init failed");

    std::string result;
    #ifdef _WIN32
        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
        std::string exe_dir = std::string(exe_path);
        exe_dir = exe_dir.substr(0, exe_dir.find_last_of("\\/"));
        std::string ca_path = exe_dir + "\\cacert.pem";
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path.c_str());
    #endif
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);   // -L
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);      // -f
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "mcpm/0.1 (libcurl)");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    if (doProgressAnimation){
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_print);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(res));

    return result;
}


std::string pb::curl_utils::url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    
    for (char c : value) {
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else if (c == ' ') {
            escaped << '%';  // encode space as %20
            escaped << "20";
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }

    return escaped.str();
}
static size_t write_file(void* ptr, size_t size, size_t nmemb, void* stream) {
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

void pb::curl_utils::curl_download_file(const std::string& url, const std::string& output_path, bool doProgressAnimation) {
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl init failed");

    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp)
        throw std::runtime_error("cannot open output file");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (doProgressAnimation){
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_print);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    }

    CURLcode res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error("download failed");
}
std::string pb::curl_utils::curl_to_string_with_http_header(std::string url, std::vector<std::string> headersVec, bool doProgressAnimation){
    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl_easy_init failed");

    std::string result;
    #ifdef _WIN32
        char exe_path[MAX_PATH];
        GetModuleFileNameA(NULL, exe_path, MAX_PATH);
        std::string exe_dir = std::string(exe_path);
        exe_dir = exe_dir.substr(0, exe_dir.find_last_of("\\/"));
        std::string ca_path = exe_dir + "\\cacert.pem";
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path.c_str());
    #endif
    struct curl_slist* headers = nullptr;
    for (const auto& h : headersVec) {
        headers = curl_slist_append(headers, h.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);   // -L
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);      // -f
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "mcpm/0.1 (libcurl)");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    if (doProgressAnimation){
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_print);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(res));

    return result;

}
