#include "Api.h"

#include <functional>
#include <array>
#include <chrono>
#include <string>


//*** BackendHandler ***

BackendHandler::BackendHandler(){
    index = std::make_shared<Index>();
    ruleFactory = std::make_shared<RuleFactory>(index);
    data = std::make_unique<TripleStorage>(index);
    rules = std::make_unique<RuleStorage>(index, ruleFactory);
}

void BackendHandler::loadData(std::string path){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new handler.");
    }
    data->read(path); 
    loadedData = true;
}

void BackendHandler::loadRules(std::string path){
    if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFormat(path, true);
    loadedRules = true;


}


std::unordered_map<std::string, int>& BackendHandler::getNodeToIdx(){
    return index->getNodeToIdx();
}
std::unordered_map<std::string, int>& BackendHandler::getRelationToIdx(){
    return index->getRelationToIdx();
}


void BackendHandler::setRuleOptions(std::map<std::string, std::string> options, RuleFactory& ruleFactory){
    

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
        {"use_u_xxd_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXd(util::stringToBool(val));}}
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

void BackendHandler::setRankingOptions(std::map<std::string, std::string> options, ApplicationHandler& ranker){
    

    // register options for ranker

     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        {"topk", [&ranker](std::string val) { ranker.setTopK(std::stoi(val)); }},
        {"num_preselect", [&ranker](std::string val) { ranker.setNumPreselect(std::stoi(val)); }},
        {"aggregation_function", [&ranker](std::string val) { ranker.setAggregationFunc(val); }},
        {"filter_w_train", [&ranker](std::string val) { ranker.setFilterWTrain(util::stringToBool(val)); }},
        {"filter_w_target", [&ranker](std::string val) { ranker.setFilterWtarget(util::stringToBool(val)); }},
        {"disc_at_least", [&ranker](std::string val) { ranker.setDiscAtLeast(std::stoi(val)); }},
        {"tie_handling", [&ranker](std::string val) { ranker.setTieHandling(val); }}
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

    void BackendHandler::subsEntityStrings(std::map<std::string, std::string>& newNames){
        index->subsEntityStrings(newNames);

    }
    void BackendHandler::subsRelationStrings(std::map<std::string, std::string>& newNames){
        index->subsRelationStrings(newNames);
    }



// ***RuleHandler***

RuleHandler::RuleHandler(std::string dataPath){
    data = std::make_unique<TripleStorage>(index);
    data->read(dataPath);
    ruleFactory = std::make_unique<RuleFactory>(index);


}


std::pair<std::vector<std::vector<std::array<std::string, 2>>>, std::vector<std::array<int,2>>> RuleHandler::calcRulesPredictions(std::vector<std::string> stringRules, bool retPredictions, bool retStats){
    //TODO: remove timing
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<std::array<std::string,2>>> preds;
    std::vector<std::array<int,2>> stats;


    if (retPredictions){
         preds.resize(stringRules.size());
    }

    if (retStats){
        stats.resize(stringRules.size());
    }

    #pragma omp parallel
    {
        #pragma omp for
        for (int i=0; i<stringRules.size(); i++){
            std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(stringRules[i]);
            rule->setTrackInMaterialize(retStats);
            if (!rule){
                throw std::runtime_error("Error in parsing rule:" + stringRules[i]);
            }
            for (auto triple : rule->materialize(*data)){
                    if (!retPredictions){
                        break; // stats are calculated with invoking materialize
                    }
                    std::string head = index->getStringOfNodeId(triple[0]);
                    std::string tail = index->getStringOfNodeId(triple[2]);
                    #pragma omp critical
                    {
                        // we know relation from the rule
                        preds[i].push_back({head ,tail});
                    }
            }

            if (retStats){
                #pragma omp critical
                {
                     stats.at(i) = rule->getStats(true);
                } 
            }

        }

    }
    // TODO remove timing
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start);
    std::cout << "Materialized "<< stringRules.size()<<" rules in "<< duration.count() << " seconds." << std::endl;
    return std::make_pair(preds, stats);
}


