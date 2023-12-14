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
        {"aggregation_function", [&ranker](std::string val) { ranker.setAggregationFunc(val); }},
        {"disc_at_least", [&ranker](std::string val) { ranker.setDiscAtLeast(std::stoi(val)); }},
        {"hard_stop_at", [&ranker](std::string val) { ranker.setNumPreselect(std::stoi(val)); }},
        {"num_top_rules", [&ranker](std::string val) {ranker.setScoreNumTopRules(std::stoi(val));}},
        {"filter_w_train", [&ranker](std::string val) { ranker.setFilterWTrain(util::stringToBool(val)); }},
        {"filter_w_target", [&ranker](std::string val) { ranker.setFilterWtarget(util::stringToBool(val)); }},
        {"tie_handling", [&ranker](std::string val) { ranker.setTieHandling(val); }},
        {"num_threads", [&ranker](std::string val) { ranker.setNumThr(std::stoi(val)); }},

    };

    //maxplus vs num_top_rules
    auto aggFunc = options.find("aggregation_function");
    auto numTopRules = options.find("num_top_rules");
    if (aggFunc != options.end() && numTopRules != options.end()) {
        if (aggFunc->second == "maxplus" && numTopRules->second != "-1") {
            std::cerr <<
             "Warning: Aggregation function is set to 'maxplus' and 'num_top_rules' is not -1. "
             "Please only do this when you know what you are doing. Otherwise set num_top_rules to -1. "
             "Current value is " 
             << numTopRules->second << std::endl;
        }
    }

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