#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "../include/curl_access.h"
#include <cstdio> // for FILE*, popen
#include <curl/curl.h>
using json = nlohmann::json;
#include "../include/mcmodm.h"
#include "../include/color.h"
std::string name;

void pb::McModm::McModm::install_mod(const std::string& pn, const json& req, bool autoPathManagement,bool just_install) {
    //set_path(install_path);
    if(!just_install){
        if (is_installed(pn)) {
            std::cout << cyan<<"[skip] Already installed: " <<yellow<< pn<<reset_color<<"\n";return;
        }

    } 
    std::string ver = req[0]["version"].get<std::string>();
    // std::cout << ver << "\n";
    std::vector<std::string> loaders;
    for (const auto& loader : req[0]["loader"]) {
        loaders.push_back(loader.get<std::string>());
        // std::cout << loader.get<std::string>() << "\n";
    }
     //std::string ver = req[0]["version"].get<std::string>();
     if (loaders.empty()) {
        throw std::runtime_error("No loaders specified in requirements.");

    }
    //std::string loader = req[0]["loader"].get<std::string>();

    // Fetch all versions for this project
    std::string url = "https://api.modrinth.com/v2/project/" + pn + "/version";
    std::string aboutVersionData;
    std::string projectUrl = "https://api.modrinth.com/v2/project/" + pn;
    std::string projectDataRaw;
    std::cout << "Fetching versions...\n";
    try {
        projectDataRaw = curl_utils::curl_to_string(projectUrl);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching main page: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }
    json projectData = json::parse(projectDataRaw);
    name = projectData["title"];
    const std::string p_type = projectData["project_type"];
    try {
        aboutVersionData= curl_utils::curl_to_string(url);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching versions: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }

    //json mainData = json::parse(aboutVersionData);
    json mainData;
    try {
        mainData = json::parse(aboutVersionData);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: [" << aboutVersionData << "]\n";
        std::cerr << "Error: " << e.what() << "\n";
        return;
    }
    bool found = false;
    bool ver_comp = false, loa_comp = false;
    for (auto& release : mainData) {
        bool ver_comp = false, loa_comp = false;
        for (auto& gver : release["game_versions"])
            if (gver == ver) { ver_comp = true; break; }
        std::string loader_to_use = "";
        for (const auto& ldr : release["loaders"]) {
            std::string rel_loader = ldr.get<std::string>();
            if (std::find(loaders.begin(), loaders.end(), rel_loader) != loaders.end()) {
                loa_comp = true;
                loader_to_use = rel_loader;
                break;
            }
            //lmn++;
        }
    

        if (ver_comp && loa_comp) {
            found = true;
            std::string dl_url = release["files"][0]["url"];
            std::string filename = release["files"][0]["filename"];
            std::string out_file;
            if (autoPathManagement){
                ProjectType ty = getProjectType(p_type);
                auto subfolder = getInstallDirectory(ty);
                //std::cout <<subfolder;
                out_file = install_path +"/"+ subfolder.string() +"/"+ filename;
            } else{
                out_file = install_path +"/"+filename;
            }

            std::cout << "Found matching version for " << name << release["name"].get<std::string>() << " (" << pn << ")"<< std::endl;
            std::cout << "Downloading to: " << out_file << "\n";

            try {
                curl_utils::curl_download_file(dl_url, out_file);
                std::cout << "\nDownload complete.\n";
            } catch (const std::exception& e) {
                std::cerr << "Download failed: " << e.what() << "\n";
            }
            if (just_install){mark_installed(pn, ver, loader_to_use, filename, name,p_type);break;} else{mark_installed(pn, ver, loader_to_use, filename, name, p_type);

            // ---- Install required dependencies ----
                if (release.contains("dependencies")) {
                    for (auto& dep : release["dependencies"]) {
                        std::string dep_type = dep["dependency_type"];
                        if (dep_type != "required") continue; // skip optional

                        std::string dep_project = dep["project_id"].get<std::string>();
                        if (is_installed(dep_project)) continue;

                        json dep_req;
                        dep_req.push_back({
                            {"version", ver},
                            {"loader", loaders}
                        });

                        install_mod(dep_project, dep_req, autoPathManagement,false);
                    }
                }
            }

            break; // stop after first matching release
        }
    
    }

    if (!found) {
        std::cout << red<<("No matching version found for " + cyan+name+red + " (" + cyan+pn+red + ")")<<reset_color<<"\n";
        if(ver_comp){
            std::cout<<cyan<<"Version is compatible with yours"<<reset_color<<"\n";
        }else if (loa_comp){
            std::cout<<cyan<<"Loader is compatible with one of yours"<<reset_color<<"\n";
        } else{std::cout<<"Nor loader nor version are compatible!"<<reset_color<<"'n";}
    }
}


