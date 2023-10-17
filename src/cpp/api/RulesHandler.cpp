#include "RulesHandler.h"


std::pair<std::vector<std::vector<std::array<std::string, 2>>>, std::vector<std::array<int,2>>> RulesHandler::calcRulesPredictions(std::vector<std::string> stringRules, bool retPredictions, bool retStats){
    std::vector<std::vector<std::array<std::string,2>>> preds;
    std::vector<std::array<int,2>> stats;

    if (!loadedData){
        throw std::runtime_error("Please load data before you calculate rule predictions/stats.");
    }


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
                        // we now relation from the rule
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
    return std::make_pair(preds, stats);
}