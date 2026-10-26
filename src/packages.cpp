// #include "../include/packages.h"
#include <fstream>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "../include/curl_access.h"
#include "../include/mcmodm.h"

using json = nlohmann::json;

//static const char* PKG_FILE = "packages.json";
/*
std::string install_path;
void set_path(const std::string& p) {
    PATH = p;                 // <-- assign to the global PATH
    if (!PATH.empty() && PATH.back() != '/'){
        PATH += '/';
    }
    PATH += "packages.json";
}*/

json pb::McModm::McModm::load_packages() {
    std::string packages_path = install_path + "/packages.json";
    std::ifstream f(packages_path);
    if (!f.is_open())
        return json{{"installed", json::object()}};

    return json::parse(f);
}

void pb::McModm::McModm::save_packages(const json& j) {
    std::ofstream f(install_path + "/packages.json");
    f << j.dump(4);
}

bool pb::McModm::McModm::is_installed(const std::string& project_id) {
    json j = load_packages();
    return j["installed"].contains(project_id);
}

void pb::McModm::McModm::mark_installed(
    const std::string& project_id,
    const std::string& game_version,
    const std::string& loader,
    const std::string& file,
    const std::string& name,
    const std::string& type,
    const int& cf_type
) {
    json j = load_packages();

    j["installed"][project_id] = {
        {"game_version", game_version},
        {"loader", loader},
        {"file", file},
        {"name", name},
        {"type", type},
        {"cf_type", cf_type}
    };

    save_packages(j);
}
bool pb::McModm::McModm::can_be_upgraded(const std::string& project_id, const std::string& game_version, const std::string& loader) {
    bool cf_mod = project_id.starts_with("cf:");
    std::string n_pid = project_id;
    std::string url = "https://api.modrinth.com/v2/project/" + n_pid + "/version";
    std::string projectUrl = "https://api.modrinth.com/v2/project/" + n_pid;
    if(cf_mod){
        n_pid = project_id.substr(3);
        url = "https://api.curseforge.com/v1/mods/" + n_pid+"/files";
        projectUrl = "https://api.curseforge.com/v1/mods/" + n_pid;
    }
    std::string verDataString,projectDataRaw;
    std::string token;
    if(cf_mod){
    std::ifstream key(pb::McModm::McModm::getConfigDir()+"api_key.txt");
    if(!key.is_open()){
        throw std::runtime_error("Could not open api_key.txt");
    }
    std::getline(key,token);

    }
    try {
        if (cf_mod) {
            projectDataRaw =
                pb::curl_utils::curl_to_string_with_http_header(
                    projectUrl,
                    {"Accept: application/json", "x-api-key: " + token},
                    false
                );

            verDataString =
                pb::curl_utils::curl_to_string_with_http_header(
                    url,
                    {"Accept: application/json", "x-api-key: " + token},
                    false
                );
        } else {
            //projectDataRaw = pb::curl_utils::curl_to_string(projectUrl);
            verDataString = pb::curl_utils::curl_to_string(url);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching project or version data: "
                  << e.what() << '\n';
        throw std::runtime_error("Failed to fetch project or version data.");
    }

    json verData;
    json projectData;

    try {
        verData = json::parse(verDataString);
    } catch (const json::parse_error& e) {
        std::cerr << "Failed to parse version data: "
                  << e.what() << '\n';
        throw std::runtime_error("Failed to parse version data.");
    }
    if(cf_mod){
        try {
            projectData = json::parse(projectDataRaw);
        } catch (const json::parse_error& e) {
            std::cerr << "Failed to parse project data: "
                      << e.what() << '\n';
            throw std::runtime_error("Failed to parse project data.");
        }
    }
    bool found = false;
    for (auto& release : (cf_mod? verData["data"]:verData)) {
        bool ver_comp = false, loa_comp = false;
        for (auto& gver : (cf_mod? release["gameVersions"]:release["game_versions"]))
            if (gver == game_version) { ver_comp = true; break; }
        if(cf_mod){
            if(pb::McModm::McModm::matchesLoader(projectData["data"], release,pb::McModm::McModm::toLower(loader))) loa_comp = true;
            
        } else{
            for (const auto& ldr : release["loaders"]) {
                std::string rel_loader = ldr.get<std::string>();
                if (ldr==loader) {
                    loa_comp = true;
                    break;
                }
                //lmn++;
            }
        }
        if (ver_comp && loa_comp) {
            found = true;
            break;
        }

    }
    return found;
}
ProjectType pb::McModm::McModm::McModm::getProjectType(const std::string& type){
    if(type=="mod"){return ProjectType::Mod;}
    else if(type=="resourcepack"){return ProjectType::ResourcePack;}
    else if(type=="modpack"){return ProjectType::ModPack;}
    else if(type=="shader"){return ProjectType::Shader;}
    return ProjectType::NONE;
}
std::filesystem::path pb::McModm::McModm::McModm::getInstallDirectory(ProjectType type) {
    switch (type) {
        case ProjectType::Mod:
            return "mods";
        case ProjectType::ResourcePack:
            return "resourcepacks";
        case ProjectType::Shader:
            return "shaderpacks";
        case ProjectType::ModPack:
            throw std::runtime_error("Modpacks are not supported");
        default:
            throw std::runtime_error("Unsupported project type");
    }
}
bool pb::McModm::McModm::verify_mod(std::string project_id, json& packages,bool apm){
    std::string pType = packages["installed"][project_id]["type"];
    std::string mod_path = install_path +"/"+pb::McModm::McModm::getInstallDirectory((project_id.starts_with("cf:")?getProjectTypeCF(packages["installed"][project_id]["cf_type"]):pb::McModm::McModm::getProjectType(pType))).string()+"/";
    mod_path+=packages["installed"][project_id]["file"];
    std::filesystem::path p(mod_path);
    return std::filesystem::exists(mod_path);
}
ProjectType pb::McModm::McModm::getProjectTypeCF(int type){
    switch(type){
        case 6:
            return ProjectType::Mod;
        case 12:
            return ProjectType::ResourcePack;
        case 6552:
            return ProjectType::Shader;
        default:
            return ProjectType::NONE;
    }
}
