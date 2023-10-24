#include "Api.h"

#include <functional>
#include <array>
#include <chrono>
#include <string>


//*** BackendHandler ***

BackendHandler::BackendHandler(){}


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


