#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "../include/mcmodm.h"
#include "../include/color.h"
using json = nlohmann::json;

void pb::McModm::McModm::verify_all_mods(json&req,InstallFlag&installFlag){
    using pb::McModm::McModm;
    bool apm = (installFlag&InstallFlag::AutoPathManagement)!=InstallFlag::None;
    json packgs = load_packages();
    std::vector<std::string> missing;
    for(const auto&[project_id, info]:packgs["installed"].items()){
        if(project_id.starts_with("local:")){
            std::cout<<"\tMod "<<cyan<<info["name"].get<std::string>()<<" ("<<project_id<<")"<<reset_color<<" is a local mod, skipping verification.\n";
            continue;
        }
        bool pack_exist = verify_mod(project_id,packgs,apm);
        std::cout<<"\tMod "<<cyan<<info["name"].get<std::string>()<<" ("<<project_id<<")"<<reset_color<<" Exists on disk: "<<(pack_exist?green+"Yes" : red+"No")<<reset_color<<"\n";
        if(!pack_exist)
            missing.push_back(project_id);
    }
    if(missing.empty()){
        std::cout<<green<<"All good!"<<reset_color<<"\n";
        return;
    }
    std::cout<<yellow<<"There are mods of which files are not on the disk.\nWhat should be done?\n[R]einstall [N]othing [U]ninstall\n (R): "<<reset_color;
    std::string wtd;
    std::getline(std::cin,wtd);
    if(wtd == "N"||wtd=="n"){std::cout<<"\nQuiting\n";return;}
    
    else if(wtd.empty()||wtd=="R"||wtd=="r"){
        std::cout<<"Reinstalling mods...\n";
        InstallFlag useIF = installFlag|InstallFlag::JustInstall;
        for(const auto&mod:missing){
            std::cout<<yellow<<"Installing "<<mod<<reset_color<<"\n";
            install_mod(mod, req,"",useIF);
        }
        std::cout<<green<<"All done!\n"<<reset_color;
    } else if(wtd=="U"||wtd=="u"){
        std::cout<<"Removing mods...\n";
        for(const auto&mod:missing){
            std::cout<<yellow<<"Removing "<<mod<<reset_color<<"\n";
            remove_package(mod, false, apm);
        }
        std::cout<<green<<"All done!\n"<<reset_color;
    } else{std::cout<<red<<"Unsupported command.\nQuitting\n"<<reset_color;}
}

