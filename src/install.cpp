#include <iostream>
#include <ostream>
#include <fstream>
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
#include <cctype>
#include <format>
std::string pb::McModm::McModm::toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    return str;
}
bool pb::McModm::McModm::McModm::matchesLoader(
    const json& project,
    const json& file,
    const std::string& requestedLoader
) {
    // CurseForge classId identifies the project category.
    const int classId = project.value("classId", 0);
    //std::cout<<"classid succ\n";

    if (requestedLoader == "paper" ||
        requestedLoader == "spigot" ||
        requestedLoader == "bukkit") {
        // Bukkit plugins use a project category, not a mod loader.
        return classId == 5;
    }
    if(requestedLoader=="iris"||requestedLoader=="optifine"){ return classId==6552;} // Shader loaders
    if(requestedLoader=="minecraft")return classId==12; // Use for resource pack

    if (!file.contains("gameVersions") ||
        !file["gameVersions"].is_array()) {
        return false;
    }

    for (const auto& version : file["gameVersions"]) {
        if (version.is_string() &&
            pb::McModm::McModm::toLower(version.get<std::string>()) == requestedLoader) {
            return true;
        }
    }

    return false;
}
std::string getExpectedHASh(const json&release,bool cf_mod){
    if(!cf_mod){
        return release["files"][0]["hashes"]["sha512"].get<std::string>(); 
    }
    std::string target;
    for(const auto&hash:release["hashes"]){
        if(hash.value("algo",0)!=1){continue;}
        target=hash.value("value","N/A");
        break;
    }
    return target;
}
void pb::McModm::McModm::install_mod(const std::string& pn, const json& req, const std::string& versionString,InstallFlag& installFlag) {
    bool cf_mod = false;
    std::string project_id = pn;
    if(pn.starts_with("cf:")){
        project_id = pn.substr(3);
        cf_mod = true;
    }
    //std::cout<<install_path<<"\n";
    bool just_install=(installFlag&InstallFlag::JustInstall)!=InstallFlag::None;
    //std::cout<<just_install<<"\n";
    bool autoPathManagement=(installFlag&InstallFlag::AutoPathManagement)!=InstallFlag::None;
    bool checkHash=(installFlag&InstallFlag::VerifyHash)!=InstallFlag::None;
    //set_path(install_path);
    bool useVersionNumber = !versionString.empty();
    if(!just_install){
        if (is_installed(pn)) {
            std::cout << cyan<<"[skip] Already installed: " <<yellow<< project_id<<reset_color<<"\n";return;
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
    std::string projectUrl, url;
    if(cf_mod){
        projectUrl = "https://api.curseforge.com/v1/mods/" + project_id;
        url = "https://api.curseforge.com/v1/mods/" + project_id+"/files";
    }else{
        projectUrl = "https://api.modrinth.com/v2/project/" + project_id;
        url = "https://api.modrinth.com/v2/project/" + project_id + "/version";
    }
    std::string aboutVersionData;
    std::string projectDataRaw;
    std::cout << "Fetching versions...\n";

    std::string token;
    if(cf_mod){
        std::ifstream key(pb::McModm::McModm::getConfigDir()+"/api_key.txt");
        if(!key.is_open()){
            throw std::runtime_error("Could not open api_key.txt");
        }
        std::getline(key,token);
    }
    try {
        if(cf_mod){
            projectDataRaw = curl_utils::curl_to_string_with_http_header(projectUrl,{"Accept: application/json","x-api-key: " + token},false);
        }else{
            projectDataRaw = curl_utils::curl_to_string(projectUrl);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching main page: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }
    json projectData = json::parse(projectDataRaw);
    if(cf_mod){name = projectData["data"]["name"];} else{name = projectData["title"];}
    std::string p_type;
    int p_type_cf;
    if(!cf_mod){
        p_type = projectData["project_type"];
    } else{
        p_type_cf = projectData["data"].value("classId",0);
    }
    try {
        if(cf_mod){
            aboutVersionData = curl_utils::curl_to_string_with_http_header(url,{"Accept: application/json","x-api-key: " + token},false);
        } else{
            aboutVersionData= curl_utils::curl_to_string(url);
        }
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
    for (auto& release : (cf_mod? mainData["data"]:mainData)) {
        bool ver_comp = false, loa_comp = false;
        std::string loader_to_use = "";
        if (useVersionNumber &&(!cf_mod)){
            if(versionString==release["version_number"].get<std::string>()){
                loa_comp = true,ver_comp = true;
                loader_to_use = release["loaders"][0].get<std::string>();
            }
        } else{
            for (auto& gver : (cf_mod? release["gameVersions"]:release["game_versions"]))
                if (gver == ver) { ver_comp = true; break; }
            if(cf_mod){
                for(const auto& local_loader:loaders){
                    if(pb::McModm::McModm::matchesLoader(projectData["data"], release,pb::McModm::McModm::toLower(local_loader))) {loa_comp = true;loader_to_use=local_loader;break;};
                }
            } else{
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
        }
    

        if (ver_comp && loa_comp) {
            found = true;
           
            std::string dl_url;
            std::string filename; 
            std::string out_file;
            if(!cf_mod){
                dl_url = release["files"][0]["url"];
                filename = release["files"][0]["filename"];
            } else{
                filename = release.at("fileName").get<std::string>();
                if (release.contains("downloadUrl") &&
                release["downloadUrl"].is_string()) {
                dl_url = release["downloadUrl"].get<std::string>();
            } else {
                // Fetch the download URL using CurseForge's dedicated endpoint.
                const std::string downloadUrlEndpoint =
                    "https://api.curseforge.com/v1/mods/" +
                    project_id + "/files/" +
                    release.at("id").get<std::string>() +
                    "/download-url";

                const std::string response =
                    curl_utils::curl_to_string_with_http_header(
                        downloadUrlEndpoint,
                        {"Accept: application/json", "x-api-key: " + token},
                        false
                    );

                const json downloadData = json::parse(response);

                if (!downloadData.contains("data") ||
                    !downloadData["data"].is_string()) {
                    throw std::runtime_error(
                        "CurseForge did not return a valid download URL."
                    );
                }

                dl_url = downloadData["data"].get<std::string>();
            }
                //dl_url = release["downloadUrl"];
                //filename = release["fileName"];
            }
            std::string expected_hash,file_hash;
            //if(!cf_mod)                                                                                                                 //FIX LATER
            expected_hash = getExpectedHASh(release, cf_mod);
            if (autoPathManagement){
                std::filesystem::path subfolder;
                if(!cf_mod){
                    ProjectType ty = getProjectType(p_type);
                    subfolder = getInstallDirectory(ty);
                } else{
                    ProjectType ty = getProjectTypeCF(p_type_cf);
                    subfolder = getInstallDirectory(ty);
                }
                //std::cout <<subfolder;
                out_file = install_path +"/"+ subfolder.string() +"/"+ filename;
            } else{
                out_file = install_path +"/"+filename;
            }

            std::cout << "Found matching version for " << name << " (" << project_id << ")"<< std::endl;
            std::cout << "Downloading to: " << out_file << "\n";

            try {
                curl_utils::curl_download_file(dl_url, out_file, file_hash,(cf_mod?"sha1":"sha512"));
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
            if(cf_mod){
                mark_installed(pn, ver, loader_to_use, filename, name,"",p_type_cf);
            } else{
                mark_installed(pn, ver, loader_to_use, filename, name,p_type);
            }
            if (just_install||((installFlag&InstallFlag::NoHandleDeps)!=InstallFlag::None)){break;} else{

            // ---- Install required dependencies ----
                if (release.contains("dependencies")) {
                    for (auto& dep : release["dependencies"]) {
                        std::string dep_project;
                        if(cf_mod){
                            std::string dep_type = dep["dependency_type"];
                            if (dep_type != "required") continue; // skip optional
                            dep_project = dep["project_id"].get<std::string>();
                        } else{
                            int dep_type = dep["relationType"];
                            if(dep_type !=3)continue;
                            dep_project = dep["modId"].get<std::string>();
                        }

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
        std::cout << red<<("No matching version found for " + cyan+name+red + " (" + cyan+project_id+red + ")")<<reset_color<<"\n";
        if(ver_comp){
            std::cout<<cyan<<"Version is compatible with yours"<<reset_color<<"\n";
        }else if (loa_comp){
            std::cout<<cyan<<"Loader is compatible with one of yours"<<reset_color<<"\n";
        } else{std::cout<<"Nor loader nor version are compatible!"<<reset_color<<"\n";}
    }
}






std::vector<Dependency> pb::McModm::McModm::get_deps(const std::string&project_id,const std::string&version_number,const json&req, const InstallFlag& installFlag){
    bool cf_mod = false;
    std::string project_id_n = project_id;
    if(project_id.starts_with("cf:")){
        project_id_n = project_id.substr(3);
        cf_mod = true;
    }

    std::vector<Dependency> deps;
    bool useVersionNumber = !version_number.empty();
    std::string game_ver = req[0]["version"].get<std::string>();
    std::vector<std::string> loaders;
    for (const auto& loader : req[0]["loader"]) {
        loaders.push_back(loader.get<std::string>());
    }
    if (loaders.empty()) {
        throw std::runtime_error("No loaders specified in requirements.");

    }
     std::string projectUrl, url;
    if(cf_mod){
        projectUrl = "https://api.curseforge.com/v1/mods/" + project_id_n;
        url = "https://api.curseforge.com/v1/mods/" + project_id_n+"/files";
    }else{
        projectUrl = "https://api.modrinth.com/v2/project/" + project_id;
        url = "https://api.modrinth.com/v2/project/" + project_id + "/version";
    }
    std::string aboutVersionData;
    std::string projectDataRaw;

    std::string token;
    if(cf_mod){
        std::ifstream key(getConfigDir()+"/api_key.txt");
        if(!key.is_open()){
            throw std::runtime_error("Could not open api_key.txt");
        }
        std::getline(key,token);
    }
    try {
        if(cf_mod){
            projectDataRaw = curl_utils::curl_to_string_with_http_header(projectUrl,{"Accept: application/json","x-api-key: " + token},false);
        }else{
            projectDataRaw = curl_utils::curl_to_string(projectUrl);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching main page: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }
    try {
        if(cf_mod){
            aboutVersionData = curl_utils::curl_to_string_with_http_header(url,{"Accept: application/json","x-api-key: " + token},false);
        }else{
            aboutVersionData = curl_utils::curl_to_string(url);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching main page: " << e.what() << "\n";
        throw std::runtime_error("Error fetching versions.");
    }
    json projectData = json::parse(projectDataRaw);
    if(cf_mod){name = projectData["data"]["name"];} else{name = projectData["title"];}
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
     for (auto& release : (cf_mod? mainData["data"]:mainData)) {
        bool ver_comp = false, loa_comp = false;
        std::string loader_to_use = "";
        if (useVersionNumber &&(!cf_mod)){
            if(version_number==release["version_number"].get<std::string>()){
                loa_comp = true,ver_comp = true;
                loader_to_use = release["loaders"][0].get<std::string>();
            }
        } else{
            for (auto& gver : (cf_mod? release["gameVersions"]:release["game_versions"]))
                if (gver == game_ver) { ver_comp = true; break; }
            if(cf_mod){
                for(const auto& local_loader:loaders){
                    if(pb::McModm::McModm::matchesLoader(projectData["data"], release,pb::McModm::McModm::toLower(local_loader))) {loa_comp = true;loader_to_use = local_loader;break;}
                    loader_to_use = local_loader;
                }
            } else{
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

        }
   

        if (ver_comp && loa_comp) {
            found = true;
            //std::cout<<"found\n";
            //std::cout << "Found matching version for " << name << release["name"].get<std::string>() << " (" << project_id << ")"<< std::endl;
            if((installFlag&InstallFlag::PushOGProject)!=InstallFlag::None)
                deps.push_back({name,project_id,DependencyType::UNKNOWN});        
            if (release.contains("dependencies")) {
                for (auto& dep : release["dependencies"]) {
                    //std::cout<<"done smth\n";
                    std::string dep_project;
                    std::string dep_name;
                    DependencyType dep_type_g;

                    if (!cf_mod) {
                        std::string dep_type = dep["dependency_type"].get<std::string>();
                        dep_type_g = get_dep_type_mr(dep_type);
                        dep_project = dep["project_id"].get<std::string>();

                        // Fetch Modrinth dependency information
                        std::string response = curl_utils::curl_to_string(
                            "https://api.modrinth.com/v2/project/" + dep_project
                        );

                        json dep_data = json::parse(response);
                        dep_name = dep_data["title"].get<std::string>();

                    } else {
                        int dep_type = dep["relationType"].get<int>();
                        dep_type_g = get_dep_type_cf(dep_type);

                        dep_project = "cf:" +
                            std::to_string(dep["modId"].get<int64_t>());

                        // Fetch CurseForge dependency information
                        std::string response =
                            curl_utils::curl_to_string_with_http_header(
                                "https://api.curseforge.com/v1/mods/" +
                                    dep_project.substr(3),
                                {"Accept: application/json", "x-api-key: " + token},
                                false
                            );

                        json dep_data = json::parse(response);
                        dep_name = dep_data["data"]["name"].get<std::string>();
                    }

                    deps.push_back({dep_name, dep_project, dep_type_g});

                    auto subdeps = get_deps(dep_project, "", req);
                    deps.insert(deps.end(), subdeps.begin(), subdeps.end());
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
    bool force = (installWrapper&InstallWrapper::Force)!=InstallWrapper::None;
    std::vector<areUpdatable>plan;
    json packgs;
    bool use_colors = false;
    if((installWrapper&InstallWrapper::NoCheckInstalled)!=InstallWrapper::None){
        packgs = load_packages();
        use_colors=true;
    }
    std::vector<Dependency> deps_plan;
    for(const auto& mod:og_plan){
        //plan.push_back(mod);
        //plan.push_back({"", mod, false});
        plan.push_back({pb::McModm::McModm::get_project_name(mod),mod,false});
        auto deps = get_deps(mod,"",req,InstallFlag::None);
        deps_plan.insert(deps_plan.end(),deps.begin(),deps.end());
    }
    std::cout<<yellow<<"Will be installed: "<<reset_color<<"\n";
    std::cout<<green<<"\tOriginal packages: "<<reset_color<<"\n";
    for(const auto&mod:plan){
        if(use_colors){
            std::cout<<"\t\t"<<(is_installed(mod.project_id)?green:yellow)<<mod.project_id<<cyan<<" ("<<mod.name<<") ";
        } else{
            std::cout<<cyan<<mod.project_id<<" ";
        }
    }
    std::cout<<reset_color<<"\n";
    std::cout<<green<<"\tDependencies: "<<reset_color<<"\n";
    for(const auto& dep:deps_plan){
        bool condition = dep.dep_type==DependencyType::REQUIRED;
        if((installWrapper&InstallWrapper::OptDeps)!=InstallWrapper::None)
            condition=condition||dep.dep_type==DependencyType::OPTIONAL;
        if(condition)std::cout<<"*";
        std::string clr=cyan;
        if(dep.dep_type==DependencyType::REQUIRED){
            clr=cyan;
        }else if(dep.dep_type==DependencyType::OPTIONAL){
            clr=green;
        }else if(dep.dep_type==DependencyType::INCOMPATIBLE){
            if(is_installed(dep.project_id)){
                clr=red;
            }else{
             clr=yellow;
            }
        }else{clr=green;}

        std::cout<<std::format("\t\t{} ({}) - {}\n", colored(is_installed(dep.project_id)?green:yellow,"{}",dep.project_id),colored(yellow,"{}",dep.name),colored(clr,"{}",dep_type_to_str(dep.dep_type)));
    }

    std::string prompt;
    std::cout << "Install these?(Y/n): ";
    std::getline(std::cin,prompt);
    if(prompt!="Y"&&prompt!="y"&&!prompt.empty()){std::cout<<"abort\n";return;}
    size_t index = 1;
    InstallFlag newFlags = installFlag|InstallFlag::NoHandleDeps;
    for(const auto& dep_mod:deps_plan){
        bool condition = dep_mod.dep_type==DependencyType::REQUIRED;
        if((installWrapper&InstallWrapper::OptDeps)!=InstallWrapper::None)
            condition=condition||dep_mod.dep_type==DependencyType::OPTIONAL;
        if(dep_mod.dep_type==DependencyType::INCOMPATIBLE){std::cout<<"W:"<<reset_color<<std::format("Incomatible dependency {} ({}) won't be installed.", dep_mod.project_id,dep_mod.name);condition=false;}
        if(dep_mod.dep_type==DependencyType::INCOMPATIBLE&&is_installed(dep_mod.project_id)&&!force){std::cerr<<red<<"E: "<< dep_mod.name<<" is incompatible with an installed mod!\n"<<reset_color;return;}
        if(condition)plan.push_back({dep_mod.name, dep_mod.project_id, false});
    }
    for(const auto&mod:plan){
        std::cout<<green<<"["<<index<<"/"<<plan.size()<<"] Installing " <<mod.project_id<<" ("<<mod.name<<")"<<reset_color<<"\n";
        install_mod(mod.project_id,req,versionString,newFlags);
        ++index;
    }
}
//Today I heard some rumor that my crush of 7 months doesn't love me back, but somebody else. Now, I am not completely believing in that, but I am kinda dissapointed,
//but if it is true, I'll get over it. - 02/11/2026 (DD/MM/YYYY)
