#include "../include/mcmodm.h"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../include/color.h"
using json = nlohmann::json;
void pb::McModm::McModm::getSetupInfo(){
    std::ifstream raw_req(install_path+"/req.json");
    if(!raw_req.is_open()){
        std::cerr<<"Could not open req.json in folder: "<<install_path<<"\n"; 
        return;
    }
    std::cout<<yellow<<"Setup info for: " <<green<< install_path<<reset_color<<"\n";
    json raw = json::parse(raw_req);
    std::string version = raw[0]["version"].get<std::string>();
    bool apm = raw[0].value("apm",false);
    std::vector<std::string> loaders;
    for(const auto& loader:raw[0]["loader"]){
        loaders.push_back(loader);
    }
    std::cout<< yellow<<"Game verion: "<< cyan<< version <<reset_color<<"\n";
    std::cout<< yellow<<"Automatic path management: "<<(apm?green+"Yes"+reset_color:red+"No"+reset_color)<<"\n";
    std::cout<<yellow<<"Loaders: \n";
    for(const auto& loader:loaders){
        std::cout<<"   "<<cyan<< loader <<reset_color<<"\n";
    }
}
