#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <cstdio> // for FILE*, popen
#include <algorithm>
//#include <curl/curl.h>
#include "../include/mcmodm.h"
#include "../include/color.h"
/*
#include "../include/packages.h"
#include "../include/remove.h"
#include "../include/updateall.h"
#include "../include/install.h"
*/
using json = nlohmann::json;
struct upgradeableModInfo{
    std::string project_id;
    json info;
    bool alreadyInstalled;
};
#include <unordered_set>
void pb::McModm::McModm::update_all_packages(std::string& version, std::vector<std::string>& loaders, json& req, std::vector<std::string> args) {
    bool force=false;
    bool checkHash=true;
    if (std::find(args.begin(), args.end(),"-f")!=args.end()||std::find(args.begin(),args.end(),"--force")!=args.end()){
        force=true;
    } 
    if(std::find(args.begin(),args.end(),"--noverify")!=args.end()){
        checkHash=false;
    }
    if (req[0]["version"] == version&&!force) {
        std::cout << "No version change detected (" << version << "). Skipping update.\n";
        return;
    }
    const bool apm = req[0].value("apm", false);
    //set_path(install_path);
    json packgs = load_packages();
    size_t packgs_instal = packgs["installed"].size();
    std::cout << "Found " << packgs_instal << " packages to upgrade.\n";
    //std::cout << packgs["installed"].dump(4) << "\n";
    bool all_updatable = true;
    //std::vector<bool> upgradibleList;
    std::unordered_set<std::string> upgradeableMods;
    std::vector<upgradeableModInfo> plan;
    for (auto& [project_id, info] : packgs["installed"].items()) {
        bool upgradable = false;
        if (project_id.starts_with("local:")){
            std::cout << "\033[33mPackages installed from a file will not be updated.\033[0m\n";
            continue;
        } else{
            upgradable = can_be_upgraded(project_id, version, info["loader"]);
            plan.push_back({project_id, info,true});
            if (force&&upgradable){upgradeableMods.insert(project_id);}
            if (!upgradable) {
                all_updatable = false;
                //std::cout << "Package " << project_id << " cannot be upgraded to version " << version << " with info[loader] " << info[loader] << ".\n";
                //continue;
            }
            }
            std::cout << "Is " << info["name"].get<std::string>() <<" (" <<project_id<< ") upgradable to version " << version << ": "<<(upgradable ? "\033[32mYes\033[0m" : "\033[31mNo\033[0m") << ".\n";
        
    }
    InstallFlag useIF = InstallFlag::JustInstall;
    useIF = useIF| (apm?InstallFlag::AutoPathManagement:InstallFlag::None);
    useIF = useIF| (checkHash?InstallFlag::VerifyHash:InstallFlag::None);
    std::cout<<yellow<<"Getting version info...\n"<<reset_color;
    if (!all_updatable) {
        if (!force){
            std::cout << "Not all packages can be upgraded. Wait for all the packages to become upgradable to the specified version " << version << ", or, run 'mcmodm ck_upd <version to update> <loader> <path to req.json>' to check that.\n";
        return;}
        std::cout << yellow<<"You are forcing this upgrade. ONLY the packages that can be upgraded will be upgraded. Run this in future to update more."<<reset_color<<std::endl;
        auto newPlan = plan;
        std::unordered_set<std::string> plannedIDs;
        for(const auto&mod:newPlan){
            plannedIDs.insert(mod.project_id);
        }
        json tio;
        tio.push_back({{"version", version}, {"loader", loaders}}); // keep the same loader(s) as before
        for(const auto& mod:plan){
            if(!upgradeableMods.contains(mod.project_id))
                continue;
            auto deps = pb::McModm::McModm::get_deps(mod.project_id,"",tio);
            std::vector<upgradeableModInfo> subplan;
            for(const auto&thingy: deps){
                if(plannedIDs.insert(thingy.project_id).second)subplan.push_back({thingy.project_id,json::array(),0});
            }
            newPlan.insert(newPlan.end(), subplan.begin(), subplan.end());
        }
        std::cout<<yellow<<"Will be installed: "<<reset_color<<"\n";
        for(const auto&mod:newPlan)
            std::cout<<(mod.alreadyInstalled?green:yellow)<<mod.project_id<<cyan<<" ("<<mod.project_id<<") ";
        std::cout<<reset_color<<"\n";
        std::string prompt;
        std::cout << "Install these?(Y/n): ";
        std::getline(std::cin,prompt);
        if(prompt!="Y"&&prompt!="y"&&!prompt.empty()){std::cout<<"abort\n";return;}
        int index = 1;
        for (auto& mod : newPlan) {
            std::cout << green << "["<<std::to_string(index)<<"/"<<std::to_string(newPlan.size())<<"] "<<"Installing " << mod.project_id << "..."<<reset_color<<"\n";
            if(is_installed(mod.project_id)){
                std::cout<<"This project is already installed.\nSkipping.\n";
                index++;
                continue;
            }
            if(mod.alreadyInstalled)
                remove_package(mod.project_id, true,apm);
            json ti;
            if(mod.alreadyInstalled){
                ti.push_back({{"version", version}, {"loader", json::array({mod.info["loader"]})}}); // keep the same loader(s) as before
            }
            install_mod(mod.project_id, (mod.alreadyInstalled?ti:tio), (const std::string)"",useIF);
            index++;
        }
    } else{
        int index = 1;
        auto newPlan = plan;
        std::unordered_set<std::string> plannedIDs;
        for(const auto&mod:newPlan)
            plannedIDs.insert(mod.project_id);
        json tio;
        tio.push_back({{"version", version}, {"loader", loaders}}); // keep the same loader(s) as before
        for(const auto& mod:plan){
            auto deps = pb::McModm::McModm::get_deps(mod.project_id,"",tio);
            std::vector<upgradeableModInfo> subplan;
            for(const auto&thingy: deps){
                if(plannedIDs.insert(thingy.project_id).second)subplan.push_back({thingy.project_id,json::array(),0});
            }
            newPlan.insert(newPlan.end(), subplan.begin(), subplan.end());
        }
        std::cout<<yellow<<"Will be installed: "<<reset_color<<"\n";
        for(const auto&mod:newPlan)
            std::cout<<(mod.alreadyInstalled?green:yellow)<<mod.project_id<<cyan<<" ("<<mod.project_id<<") ";
        std::cout<<reset_color<<"\n";
        std::string prompt;
        std::cout << "Install these?(Y/n): ";
        std::getline(std::cin,prompt);
        if(prompt!="Y"&&prompt!="y"&&!prompt.empty()){std::cout<<"abort\n";return;}
        for (auto& mod:newPlan) {
                std::cout << green << "["<<std::to_string(index)<<"/"<<std::to_string(newPlan.size())<<"] "<<"Installing " << mod.project_id << "..."<<reset_color<<"\n";
                bool up_to_date = false;
                if(mod.alreadyInstalled){
                    if(mod.info["game_version"] == version){
                        std::cout << "Package " << mod.project_id << " is already up to date for version " << version << ". Skipping.\n";
                        index++;
                        up_to_date = true;
                        continue;
                    }
                }
                if(!up_to_date){
                    if(mod.alreadyInstalled)
                        remove_package(mod.project_id, true,apm);
                    json ti;
                    if(mod.alreadyInstalled){
                        ti.push_back({{"version", version}, {"loader", json::array({mod.info["loader"]})}}); // keep the same loader(s) as before
                    }
                    install_mod(mod.project_id, (mod.alreadyInstalled?ti:tio), (const std::string)"",useIF);

                    //continue;
                    index++;
                }
            
        }
    }
    std::string req_path = install_path;
    if (!req_path.empty() && req_path.back() != '/')
        req_path += '/';
    req_path += "req.json";
    json req_s;
    req_s.push_back({{"version", version}, {"loader", loaders}});
    std::ofstream ofs(req_path);
    if (!ofs.is_open()) {
        std::cerr << "Failed to write req.json to " << req_path << "\n";
        return;
    }
    ofs << req_s.dump(4);
    ofs.close();
    std::cout << "Updated req.json with new version " << version << ".\n";
}
std::vector<areUpdatable> pb::McModm::McModm::check_all_upgradeable(std::string& version, std::string& loader){
    //set_path(install_path);
    json packgs = load_packages();
    //size_t packgs_instal = packgs["installed"].size();
    std::vector<areUpdatable> updatables;

    for (auto& [project_id, info] : packgs["installed"].items()) {
        if(project_id.starts_with("local:")){std::cout<< "Package " << project_id << " Is a local package. Skipping.\n";continue;}
        bool upgradable = can_be_upgraded(project_id, version, info["loader"]);
        updatables.push_back({info["name"].get<std::string>(), project_id, upgradable});
        //std::cout << "Is " << info["name"].get<std::string>() <<" (" <<project_id<< ") upgradable to version " << version << "and loader" << info["loader"]<<": "<<(upgradable ? "\033[32mYes\033[0m" : "\033[31mNo\033[0m") << ".\n";
    }
    return updatables;
}
