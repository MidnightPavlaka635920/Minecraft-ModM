#include "../include/packages.h"
#include <fstream>
#include <nlohmann/json.hpp>

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
