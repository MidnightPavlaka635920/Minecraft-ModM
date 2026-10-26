#include <unistd.h>
#include <openssl/evp.h>
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
#include <chrono>
#include <algorithm>
#include "../include/curl_access.h"
/*size_t progress_print(void* ptr, curl_off_t total, curl_off_t now, curl_off_t, curl_off_t){
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
}*/
struct ProgressData {
    curl_off_t lastBytes = 0;
    std::chrono::steady_clock::time_point lastTime =
        std::chrono::steady_clock::now();
    double speed = 0.0;
};

static void format_size(double bytes, char* out, size_t size) {
    const char* units[] = {"B", "K", "M", "G", "T"};
    int unit = 0;

    while (bytes >= 1024.0 && unit < 4) {
        bytes /= 1024.0;
        ++unit;
    }

    if (unit == 0)
        snprintf(out, size, "%.0f %s", bytes, units[unit]);
    else
        snprintf(out, size, "%.1f %sB", bytes, units[unit]);
}

int progress_print(
    void* ptr,
    curl_off_t total,
    curl_off_t now,
    curl_off_t,
    curl_off_t
) {
    auto* p = static_cast<ProgressData*>(ptr);

    auto currentTime = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(
        currentTime - p->lastTime
    ).count();

    if (elapsed >= 0.5) {
        p->speed = std::max(
            0.0,
            static_cast<double>(now - p->lastBytes) / elapsed
        );

        p->lastBytes = now;
        p->lastTime = currentTime;
    }

    constexpr int barWidth = 50;
    double fraction = total > 0
        ? std::clamp(
            static_cast<double>(now) / total, 0.0, 1.0
          )
        : 0.0;

    int filled = static_cast<int>(barWidth * fraction);

    char downloaded[32], totalSize[32], speed[32];
    format_size(static_cast<double>(now), downloaded,
                sizeof(downloaded));
    format_size(static_cast<double>(total), totalSize,
                sizeof(totalSize));
    format_size(p->speed, speed, sizeof(speed));

    std::cout << "\r[";

    for (int i = 0; i < barWidth; ++i)
        std::cout << (i < filled ? "#" : "-");

    std::cout << "] ";

    if (total > 0)
        std::cout << std::fixed << std::setprecision(1)
                  << fraction * 100.0 << "% ";
    else
        std::cout << "??.?% ";

    std::cout << downloaded << " / ";

    if (total > 0)
        std::cout << totalSize;
    else
        std::cout << "unknown";

    std::cout << "  " << speed << "/s" << std::flush;

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
    //curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);      // -f
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "mcpm/0.1 (libcurl)");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    ProgressData progress;

    if (doProgressAnimation) {
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
                         progress_print);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }
    CURLcode res = curl_easy_perform(curl);

    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(
            std::string("curl error: ") + curl_easy_strerror(res)
        );
    }

    if (response_code == 404) {
        throw std::runtime_error("HTTP 404: Not found");
    }

    if (response_code >= 400) {
        throw std::runtime_error(
            "HTTP error: " + std::to_string(response_code)
        );
    }

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
    auto* ctx= static_cast<DownloadData*>(stream);
    size_t total = size*nmemb;
    if (fwrite(ptr, 1, total, ctx->file) != total)
        return 0;
    if (ctx->hash && EVP_DigestUpdate(ctx->hash, ptr, total) != 1)
        return 0;
    return total;
}
void pb::curl_utils::curl_download_file(const std::string& url, const std::string& output_path, std::string& hash_string, std::string algo,bool doProgressAnimation) {
    const EVP_MD* md = nullptr;

    if (algo == "sha512")
        md = EVP_sha512();
    else if (algo == "sha1")
        md = EVP_sha1();
    else if (algo != "none")
        throw std::invalid_argument("Unsupported hash algorithm: " + algo);

    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl init failed");

    FILE* fp = fopen(output_path.c_str(), "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("cannot open output file");
    }

    EVP_MD_CTX* hash = nullptr;

    if (md) {
        hash = EVP_MD_CTX_new();

        if (!hash || EVP_DigestInit_ex(hash, md, nullptr) != 1) {
            EVP_MD_CTX_free(hash);
            fclose(fp);
            curl_easy_cleanup(curl);
            throw std::runtime_error("hash initialization failed");
        }
    }

    DownloadData ctx{
        .file = fp,
        .hash = hash
    };

#ifdef _WIN32
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);

    std::string exe_dir(exe_path);
    exe_dir = exe_dir.substr(0, exe_dir.find_last_of("\\/"));

    std::string ca_path = exe_dir + "\\cacert.pem";
    curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path.c_str());
#endif

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    ProgressData progress;

    if (doProgressAnimation) {
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
                         progress_print);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }

    CURLcode res = curl_easy_perform(curl);

    if (doProgressAnimation)
        std::cout << '\n';

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);

        curl_easy_cleanup(curl);
        fclose(fp);
        EVP_MD_CTX_free(hash);

        throw std::runtime_error("download failed: " + error);
    }

    if (hash) {
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int digest_len = 0;

        if (EVP_DigestFinal_ex(hash, digest, &digest_len) != 1) {
            curl_easy_cleanup(curl);
            fclose(fp);
            EVP_MD_CTX_free(hash);
            throw std::runtime_error("hash finalization failed");
        }

        std::ostringstream ss;

        for (unsigned int i = 0; i < digest_len; ++i) {
            ss << std::hex
               << std::setw(2)
               << std::setfill('0')
               << static_cast<unsigned int>(digest[i]);
        }

        hash_string = ss.str();
    } else {
        hash_string.clear();
    }

    fclose(fp);
    curl_easy_cleanup(curl);
    EVP_MD_CTX_free(hash);
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
    ProgressData progress;

    if (doProgressAnimation) {
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION,
                         progress_print);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }

    CURLcode res = curl_easy_perform(curl);

       long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        std::cout<<response_code<<"\n";
        throw std::runtime_error(
            std::string("curl error: ") + curl_easy_strerror(res)+" "+std::to_string(response_code)
        );
    }

    if (response_code == 404) {
        throw std::runtime_error("HTTP 404: Not found");
    }

    if (response_code >= 400) {
        throw std::runtime_error(
            "HTTP error: " + std::to_string(response_code)
        );
    }
    curl_easy_cleanup(curl);
    return result;

}
