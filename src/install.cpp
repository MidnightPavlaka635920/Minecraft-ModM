#include <iostream>
#include <ostream>
#include <string>
#include <strings.h>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "../include/curl_access.h"
#include <curl/curl.h>
using json = nlohmann::json;
#include "../include/mcmodm.h"
#include "../include/color.h"
std::string name;

void pb::McModm::McModm::install_mod(const std::string& pn, const json& req, const std::string& versionString,InstallFlag& installFlag) {
    //std::cout<<install_path<<"\n";
    bool just_install=(installFlag&InstallFlag::JustInstall)!=InstallFlag::None;
    //std::cout<<just_install<<"\n";
    bool autoPathManagement=(installFlag&InstallFlag::AutoPathManagement)!=InstallFlag::None;
    bool checkHash=(installFlag&InstallFlag::VerifyHash)!=InstallFlag::None;
    //set_path(install_path);
    bool useVersionNumber = !versionString.empty();
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
        std::string loader_to_use = "";
        if (useVersionNumber){
            if(versionString==release["version_number"].get<std::string>()){
                loa_comp = true,ver_comp = true;
                loader_to_use = release["loaders"][0].get<std::string>();
            }
        } else{
            for (auto& gver : release["game_versions"])
                if (gver == ver) { ver_comp = true; break; }
            for (const auto& ldr : release["loaders"]) {
                std::string rel_loader = ldr.get<std::string>();
                if (std::find(loaders.begin(), loaders.end(), rel_loader) != loaders.end()) {
                    loa_comp = true;
                    loader_to_use = rel_loader;
                    break;
                }
                //lmn++;
            }
        }
    

        if (ver_comp && loa_comp) {
            found = true;
            std::string dl_url = release["files"][0]["url"];
            std::string filename = release["files"][0]["filename"];
            std::string out_file;
            std::string expected_hash,file_hash;
            expected_hash = release["files"][0]["hashes"]["sha512"];
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
                curl_utils::curl_download_file(dl_url, out_file, file_hash);
                std::cout << "\nDownload complete.\n";
            } catch (const std::exception& e) {
                std::cerr << "Download failed: " << e.what() << "\n";
            }
            if(checkHash){
                if(file_hash!=expected_hash){
                    std::cout<<red<<"Hashes are not matching!\nAboritng!"<<reset_color;
                    std::cout<<"Expected hash: "<<expected_hash<<"\n";
                    std::cout<<"File Hash:     "<<file_hash<<"\n";
                    return;
                }
            }
            mark_installed(pn, ver, loader_to_use, filename, name,p_type);
            if (just_install||((installFlag&InstallFlag::NoHandleDeps)!=InstallFlag::None)){break;} else{

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
                        std::string vs = "";
                        InstallFlag tempIF = InstallFlag::None;
                        tempIF = tempIF| (checkHash?InstallFlag::VerifyHash: InstallFlag::None);
                        tempIF = tempIF| (autoPathManagement?InstallFlag::AutoPathManagement:InstallFlag::None);
                        install_mod(dep_project, dep_req, vs,tempIF);
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
        } else{std::cout<<"Nor loader nor version are compatible!"<<reset_color<<"\n";}
    }
}






std::vector<areUpdatable> pb::McModm::McModm::get_deps(const std::string&project_id,const std::string&version_number,const json&req, const InstallFlag& installFlag){
    std::vector<areUpdatable> deps;
    bool useVersionNumber = !version_number.empty();
    std::string game_ver = req[0]["version"].get<std::string>();
    std::vector<std::string> loaders;
    for (const auto& loader : req[0]["loader"]) {
        loaders.push_back(loader.get<std::string>());
    }
    if (loaders.empty()) {
        throw std::runtime_error("No loaders specified in requirements.");

    }
    std::string url = "https://api.modrinth.com/v2/project/" + project_id + "/version";
    std::string aboutVersionData;
    std::string projectUrl = "https://api.modrinth.com/v2/project/" + project_id;
    std::string projectDataRaw;
    //std::cout << "Fetching versions...\n";
    try {
        projectDataRaw = pb::curl_utils::curl_to_string(projectUrl);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching main page: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }
    json projectData = json::parse(projectDataRaw);
    name = projectData["title"];
    if((installFlag&InstallFlag::PushOGProject)!=InstallFlag::None){
        deps.push_back({name,project_id,true});
    }
    const std::string p_type = projectData["project_type"];
    try {

        aboutVersionData= pb::curl_utils::curl_to_string(url);
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
        throw std::runtime_error("JSON parsing failed.");
    }
    bool found = false;
    bool ver_comp = false, loa_comp = false;
    for (auto& release : mainData) {
        bool ver_comp = false, loa_comp = false;
        std::string loader_to_use = "";
        if (useVersionNumber){
            if(version_number==release["version_number"].get<std::string>()){
                loa_comp = true,ver_comp = true;
                loader_to_use = release["loaders"][0].get<std::string>();
            }
        } else{
            for (auto& gver : release["game_versions"])
                if (gver == game_ver) {ver_comp = true; break; }
            for (const auto& ldr : release["loaders"]) {
                std::string rel_loader = ldr.get<std::string>();
                if (std::find(loaders.begin(), loaders.end(), rel_loader) != loaders.end()) {
                    loa_comp = true;
                    loader_to_use = rel_loader;
                    break;
                }
                //lmn++;
            }
        }
    

        if (ver_comp && loa_comp) {
            found = true;
            //std::cout << "Found matching version for " << name << release["name"].get<std::string>() << " (" << project_id << ")"<< std::endl;
            //deps.push_back(project_id);        
                if (release.contains("dependencies")) {
                    for (auto& dep : release["dependencies"]) {
                        std::string dep_type = dep["dependency_type"];
                        if (dep_type != "required") continue; // skip optional

                        std::string dep_project = dep["project_id"].get<std::string>();
                        json dep_data;
                        try{
                            std::string dep_name = pb::curl_utils::curl_to_string("https://api.modrinth.com/v2/project/" + dep_project);
                            dep_data = json::parse(dep_name);
                        } catch (const std::exception& e) {
                            std::cerr << "Error fetching dependency project name: " << e.what() << "\n";
                            throw std::runtime_error("Error fetching dependency project name.");
                        }
                        deps.push_back({dep_data["title"].get<std::string>(),dep_project, true});
                        auto subdeps =get_deps(dep_project,"",req);
                        deps.insert(deps.end(),subdeps.begin(),subdeps.end());
                    }
                }
                break; // stop after first matching release
            }
    }
    if(!found) {
        std::cout << red<<("No matching version found for " + cyan+name+red + " (" + cyan+project_id+red + ")")<<reset_color<<"\n";
        if(ver_comp){
            std::cout<<cyan<<"Version is compatible with yours"<<reset_color<<"\n";
        }else if (loa_comp){
            std::cout<<cyan<<"Loader is compatible with one of yours"<<reset_color<<"\n";
        } else{std::cout<<"Nor loader nor version are compatible!"<<reset_color<<"\n";}
        throw std::runtime_error("No matching version found.");
    }
    return deps;
}

void pb::McModm::McModm::install_wrapper(const std::vector<std::string>og_plan, const json& req, const std::string& versionString,InstallFlag& installFlag,const InstallWrapper& installWrapper) {
    std::vector<areUpdatable>plan;
    //std::ifstream packgs(install_path+"/packages.json");
    //if(!packgs.is_open()){
    //    std::cerr<<red<<"Could not open packages.json in folder: "<<install_path<<"\n"<<reset_color; 
    //    throw std::runtime_error("Could not open packages.json");
    //}
    json packgs;//load_packages();
    bool use_colors = false;
    if((installWrapper&InstallWrapper::NoCheckInstalled)!=InstallWrapper::None){
        packgs = load_packages();
        use_colors=true;
    }
    for(const auto& mod:og_plan){
        //plan.push_back(mod);
        auto deps = get_deps(mod,"",req,InstallFlag::PushOGProject);
        plan.insert(plan.end(),deps.begin(),deps.end());
    }
    std::cout<<yellow<<"Will be installed: "<<reset_color<<"\n";
    for(const auto&mod:plan){
        if(use_colors){
            std::cout<<(is_installed(mod.project_id)?green:yellow)<<mod.project_id<<cyan<<" ("<<mod.name<<") ";
        } else{
            std::cout<<cyan<<mod.project_id<<" ";
        }
    }
    std::cout<<reset_color<<"\n";
    std::string prompt;
    std::cout << "Install these?(Y/n): ";
    std::getline(std::cin,prompt);
    if(prompt!="Y"&&prompt!="y"&&!prompt.empty()){std::cout<<"abort\n";return;}
    size_t index = 1;
    InstallFlag newFlags = installFlag|InstallFlag::NoHandleDeps;
    for(const auto&mod:plan){
        std::cout<<green<<"["<<index<<"/"<<plan.size()<<"] Installing " <<mod.project_id<<" ("<<mod.name<<")"<<reset_color<<"\n";
        install_mod(mod.project_id,req,versionString,newFlags);
        ++index;
    }
}
