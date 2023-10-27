#include "DataHandler.h"

#include <functional>


 DataHandler::DataHandler(std::map<std::string, std::string> options){
    index = std::make_shared<Index>();
    data = std::make_unique<TripleStorage>(index);
    target = std::make_unique<TripleStorage>(index);
    filter = std::make_unique<TripleStorage>(index);
    ruleFactory = std::make_shared<RuleFactory>(index);
    setRuleOptions(options, *ruleFactory);
    rules = std::make_unique<RuleStorage>(index, ruleFactory);
 }


void DataHandler::loadData(std::string path){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }
    data->read(path); 
    loadedData = true;
}

void DataHandler::loadData(std::string path, std::string filterPath){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }
    data->read(path); 

    if (filterPath!=""){
        filter->read(filterPath);
    }
    loadedData = true;
}

bool DataHandler::getLoadedData(){
    return loadedData;
}

bool DataHandler::getLoadedRules(){
    return loadedRules;
}


void DataHandler::loadDatasets(std::string targetPath, std::string trainPath, std::string filterPath){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }

    std::cout<<"Load data... \n";


    target->read(targetPath);
    if (filterPath!=""){
        filter->read(filterPath);
    }
    
    // sets loadedData to true
    loadData(trainPath);
}


void DataHandler::setOptions(std::map<std::string, std::string> options){
    setRuleOptions(options, *ruleFactory);
}


void DataHandler::loadRules(std::string path){
    if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFormat(path, true);
    loadedRules = true;


}

std::unordered_map<std::string, int>& DataHandler::getNodeToIdx(){
    return index->getNodeToIdx();
}
std::unordered_map<std::string, int>& DataHandler::getRelationToIdx(){
    return index->getRelationToIdx();
}


void DataHandler::subsEntityStrings(std::map<std::string, std::string>& newNames){
        index->subsEntityStrings(newNames);

    }
void DataHandler::subsRelationStrings(std::map<std::string, std::string>& newNames){
        index->subsRelationStrings(newNames);
    }



void DataHandler::setRuleOptions(std::map<std::string, std::string> options, RuleFactory& ruleFactory){
    

    // rule options:  individual rule options and options of which rules to use
     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        {"rule_zero_weight", [this](std::string val) { RuleZ::zConfWeight = std::stod(val); }},
        {"rule_u_d_weight", [this](std::string val) { RuleD::dConfWeight = std::stod(val); }},
        {"rule_b_max_branching_factor", [this](std::string val) { RuleB::branchingFaktor = std::stoi(val); }},
        {"use_zero_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleZ(util::stringToBool(val));}},
        {"use_u_c_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleC(util::stringToBool(val));}},
        {"use_b_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleB(util::stringToBool(val));}},
        {"use_u_d_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleD(util::stringToBool(val));}},
        {"use_u_xxc_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXc(util::stringToBool(val));}},
        {"use_u_xxd_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXd(util::stringToBool(val));}},
        {"rule_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val));}}
    };

    for (auto& handler : handlers) {
        auto opt = options.find(handler.name);
        if (opt != options.end()) {
            if (verbose){
                std::cout<< "Setting option "<<handler.name<<" to: "<<opt->second<<std::endl;
            }
            handler.setter(opt->second);
        }
    }
}


    TripleStorage& DataHandler::getData(){
        return *data;
    }
    TripleStorage& DataHandler::getFilter(){
        return *filter;
    }
    TripleStorage& DataHandler::getTarget(){
        return *target;

    }
    RuleStorage& DataHandler::getRules(){
        return *rules;
    }

    RuleFactory& DataHandler::getRuleFactory(){
        return *ruleFactory;
    }


    std::shared_ptr<Index> DataHandler::getIndex(){
        return index;
    }


