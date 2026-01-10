#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "../include/curl_access.h"
#include <cstdio> // for FILE*, popen
#include <curl/curl.h>
using json = nlohmann::json;
#include "../include/packages.h"

void install_mod(const std::string& pn, const std::string& install_path, const json& req, bool just_install) {
    set_path(install_path);
    if(just_install){}
    else{
        if (is_installed(pn)) {
            std::cout << "[skip] Already installed: " << pn << "\n";
            return;
        }
    }

    std::string ver = req[0]["version"].get<std::string>();
    std::string loader = req[0]["loader"].get<std::string>();

    // Fetch all versions for this project
    std::string url = "https://api.modrinth.com/v2/project/" + pn + "/version";
    std::string oVerData;
    std::string nurl = "https://api.modrinth.com/v2/project/" + pn;
    std::string ntf;
    try {
        ntf = curl_to_string(nurl);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching main page: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }
    json pdata = json::parse(ntf);
    try {
        oVerData = curl_to_string(url);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching versions: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }

    //json mainData = json::parse(oVerData);
    json mainData;
    try {
        mainData = json::parse(oVerData);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: [" << oVerData << "]\n";
        std::cerr << "Error: " << e.what() << "\n";
        return;
    }
    bool found = false;

    for (auto& release : mainData) {
        bool ver_comp = false, loa_comp = false;

        for (auto& gver : release["game_versions"])
            if (gver == ver) { ver_comp = true; break; }

        for (auto& ldr : release["loaders"])
            if (ldr == loader) { loa_comp = true; break; }

        if (ver_comp && loa_comp) {
            found = true;

            std::string dl_url = release["files"][0]["url"];
            std::string filename = release["files"][0]["filename"];
            std::string out_file = install_path + "/" + filename;
            std::string name = pdata["title"];

            std::cout << "Found matching version for " << pn << release["name"] << std::endl;
            std::cout << "Downloading to: " << out_file << "\n";

            try {
                curl_download_file(dl_url, out_file);
                std::cout << "Download complete.\n";
            } catch (const std::exception& e) {
                std::cerr << "Download failed: " << e.what() << "\n";
            }
            if (just_install){mark_installed(pn, ver, loader, filename, name);continue;} else{mark_installed(pn, ver, loader, filename, name);

            // ---- Install required dependencies ----
                if (release.contains("dependencies")) {
                    for (auto& dep : release["dependencies"]) {
                        std::string dep_type = dep["dependency_type"];
                        if (dep_type != "required") continue; // skip optional

                        std::string dep_project = dep["project_id"];
                        if (is_installed(dep_project)) continue;

                        json dep_req;
                        dep_req.push_back({ {"version", ver}, {"loader", loader} });

                        install_mod(dep_project, install_path, dep_req, false);
                    }
                }
            }

            break; // stop after first matching release
        }
    }

    if (!found) {
        throw std::runtime_error("No matching version found for " + pn);
    }
}