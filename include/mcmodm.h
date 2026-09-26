#pragma once
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>
using json = nlohmann::json;
enum class ProjectType{Mod, ModPack, ResourcePack, Shader, NONE};
struct search_result {
    std::string title;
    std::string author;
    std::string project_id;
};

struct areUpdatable{
    std::string name;
    std::string project_id;
    bool updatable;
};
struct list_info {
    std::string name;
    std::string project_id;
    std::string game_version;
};

struct ModInfo {
    std::string name;
    std::vector<std::string> authors;
    std::string description;
    std::string project_type;
    std::string project_id;
    bool ss = false;
    bool cs = false;
};
struct version_names_info {
    std::string ver_info;
    std::unordered_set<std::string> game_ver;
};
namespace pb::McModm{

class McModm{
    private:
        std::string install_path;
    public:
        // Construct a manager bound to a specific installation path
        McModm(std::string path): install_path(path){};

        //search.h
        // Search for mods/plugins online by query (static)
        static std::vector<search_result> search_mods(const std::string& query);

        //packages.h
        // Load the installed packages information from disk
        json load_packages();
        // Save the installed packages information to disk
        void save_packages(const json& j);
        // Check if a package can be upgraded to a specific game version and loader
        static bool can_be_upgraded(const std::string& project_id, const std::string& game_version, const std::string& loader);
        // Check if a package is already installed
        bool is_installed(const std::string& project_id);
        // Mark a package as installed in the packages database
        void mark_installed(
            const std::string& project_id,
            const std::string& game_version,
            const std::string& loader,
            const std::string& file,
            const std::string& name,
            const std::string& type
        );
        // Resolve the package project type from its string identifier
        static ProjectType getProjectType(const std::string& type);
        // Get the installation directory for a project type
        static std::filesystem::path getInstallDirectory(ProjectType type);
        // Check whether a specific installed mod still exists on disk and matches the expected package metadata
        bool verify_mod(std::string project_id, json& packages,bool apm);
        //install.cpp
        // Install a mod/plugin by project ID and requirements
        void install_mod(const std::string& pn, const json& req, bool autoPathManagement, const std::string& versionString,bool just_install);
        //remove.cpp
        // Remove a package by project ID
        void remove_package(std::string package_id, bool just_remove, bool autoPathHandling);
        //updateall.cpp
        // Update all installed packages to a new game version and loaders
        void update_all_packages(std::string& version, std::vector<std::string>& loaders, json& req, bool force = false);
        // Check if all packages are upgradeable to a specific version and loader
        std::vector<areUpdatable> check_all_upgradeable(std::string& version, std::string& loader);
        //list.cpp
        // List all installed packages
        std::vector<list_info> list_packs();
        //easy.cpp
        // Interactive easy install mode for mods/plugins
        void easy_install(bool color);
        // Interactive easy remove mode for mods/plugins
        void easy_remove(bool color);
        //iff.cpp
        // Install all packages from a packages.json file
        void iff(const std::string& packages_path);
        //il.cpp
        // Install a local file as a mod/plugin
        void install_local(std::string& path_tif, std::string& name, std::string& version ,std::string& loader, std::string& type);
        //ck_vers.cpp
        // List all compatible versions for a project (static)
        static std::vector<std::string> list_compatible_versions(std::string project_id, const std::string& loader = "");
        // List loaders compatible with a specific project version
        static std::vector<std::string> list_comp_loaders(std::string& version, std::string& project_id);
        // List all known version numbers for a project, optionally filtered by game version
        static std::vector<version_names_info> list_version_nums(const std::string& project_id, const std::string& game_version);
        //info.cpp
        // Get detailed info for a project (static)
        static std::vector<ModInfo> mod_info(const std::string& query);
        // Print the current setup information for the configured installation path
        void getSetupInfo();
        //instances.cpp
        // Resolve a path or instance to an actual installation directory
        static std::string getPath(std::string& pathString, std::string& instanceString);
        // Get all saved named instances and their target directories
        static std::unordered_map<std::string, std::string> getInstances();
        // Save a named instance pointing to a target directory
        static void addInstance(std::string& path, std::string& name);
        // Remove a named instance by its identifier
        static void removeInstance(std::string& name);
        //verify_installed.cpp
        // Verify that all installed mods still exist on disk and prompt for missing-file recovery actions
        void verify_all_mods(bool apm,json&req);
};
// Setup a managed installation directory with the given version and loader list
void setup(std::string& path, std::string& version, std::vector<std::string>& loaders);

}
