#include "../include/packages.h"
#include <fstream>
#include <string>
#include <iostream>
#include <nlohmann/json.hpp>
#include "../include/curl_access.h"

using json = nlohmann::json;

//static const char* PKG_FILE = "packages.json";
std::string PATH;
void set_path(const std::string& p) {
    PATH = p;                 // <-- assign to the global PATH
    if (!PATH.empty() && PATH.back() != '/'){
        PATH += '/';
    }
    PATH += "packages.json";
}

json load_packages() {
    std::ifstream f(PATH);
    if (!f.is_open())
        return json{{"installed", json::object()}};

    return json::parse(f);
}

void save_packages(const json& j) {
    std::ofstream f(PATH);
    f << j.dump(4);
}

bool is_installed(const std::string& project_id) {
    json j = load_packages();
    return j["installed"].contains(project_id);
}

void mark_installed(
    const std::string& project_id,
    const std::string& game_version,
    const std::string& loader,
    const std::string& file,
    const std::string& name
) {
    json j = load_packages();

    j["installed"][project_id] = {
        {"game_version", game_version},
        {"loader", loader},
        {"file", file},
        {"name", name}
    };

    save_packages(j);
}
bool can_be_upgraded(const std::string& project_id, const std::string& game_version, const std::string& loader) {
    std::string url = "https://api.modrinth.com/v2/project/" + project_id + "/version";
    std::string verDataString;
    try {
        verDataString = curl_to_string(url);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching versions: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions."); 
    }
    json verData;
    try{
        verData = json::parse(verDataString);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: [" << verDataString << "]\n";
        std::cerr << "Error: " << e.what() << "\n";
        throw std::runtime_error("Failed to parse version data.");
    }
    bool found = false;
    for (auto& release : verData) {
        bool ver_comp = false, loa_comp = false;

        for (auto& gver : release["game_versions"])
            if (gver == game_version) { ver_comp = true; break; }

        for (auto& ldr : release["loaders"])
            if (ldr == loader) { loa_comp = true; break; }

        if (ver_comp && loa_comp) {
            found = true;
            break;
        }

    }
    return found;
}