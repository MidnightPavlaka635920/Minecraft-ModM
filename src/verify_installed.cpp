#include <iostream>
#include <nlohmann/json.hpp>
#include "../include/mcmodm.h"
#include "../include/color.h"
using json = nlohmann::json;

void pb::McModm::McModm::verify_all_mods(bool apm){
    using pb::McModm::McModm;
    json packgs = load_packages();
    std::vector<bool> exist;
    for(const auto&[project_id, info]:packgs["installed"].items()){
        bool pack_exist = verify_mod(project_id,packgs,apm);
        std::cout<<"\tMod "<<cyan<<info["name"].get<std::string>()<<" ("<<project_id<<")"<<reset_color<<" Exists on disk: "<<(pack_exist?green+"Yes" : red+"No")<<reset_color<<"\n";
        exist.push_back(pack_exist);
    }
}

