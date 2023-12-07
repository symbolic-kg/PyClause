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
        {"tie_handling", [&ranker](std::string val) { ranker.setTieHandling(val); }},
        {"num_threads", [&ranker](std::string val) { ranker.setNumThr(std::stoi(val)); }},

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