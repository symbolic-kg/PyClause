#include "RankingHandler.h"


#include <string>
#include <map>
#include <functional>


RankingHandler::RankingHandler(std::map<std::string, std::string> options): BackendHandler(){
    setRankingOptions(options, ranker);
    setOptions(options);
}


void RankingHandler::setOptions(std::map<std::string, std::string> options){
    // register options for ranker

     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };
    
    std::vector<OptionHandler> handlers = {
        {"collect_rules", [this](std::string val) {this->setCollectRules(util::stringToBool(val));}},
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


void RankingHandler::calculateRanking(std::shared_ptr<DataHandler> dHandler){
    index = dHandler->getIndex();
    ranker.clearAll();
    if (collectRules){
        ranker.setSaveCandidateRules(true);
        // bind lifetime of rules to this object
        myDhandler = dHandler;
    }
    ranker.makeRanking(dHandler->getTarget(), dHandler->getData(), dHandler->getRules(),dHandler->getFilter());
}


void RankingHandler::writeRanking(std::string writePath, std::shared_ptr<DataHandler> dHandler){
    ranker.writeRanking(dHandler->getTarget(), writePath);

}


std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> RankingHandler::getRanking(std::string headOrTail){
    if (headOrTail=="head"){
        return ranker.getHeadQcandsConfs();
    }else if (headOrTail=="tail"){
        return ranker.getTailQcandsConfs();
    }else{
        throw std::runtime_error("Please specify 'head' or 'tail' as first argument of getRanking");
    }
}


 std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>>RankingHandler::getStrRanking(std::string headOrTail) {
    auto idxRanking = (headOrTail == "head") ? ranker.getHeadQcandsConfs() : ranker.getTailQcandsConfs();
    if (!(headOrTail =="head") && !(headOrTail =="tail")){
        throw std::runtime_error("Please specify 'head' or 'tail' as first argument of getRanking");
    }
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>> strRanking;
    for (const auto& outer_pair : idxRanking) {
        const std::string outerKeyStr = index->getStringOfRelId(outer_pair.first);
        for (const auto& middle_pair : outer_pair.second) {
            const std::string middleKeyStr = index->getStringOfNodeId(middle_pair.first);
            std::vector<std::pair<std::string, double>>& pairsVec = strRanking[outerKeyStr][middleKeyStr];
            for (const auto& inner_pair : middle_pair.second) {
                const std::string innerKeyStr = index->getStringOfNodeId(inner_pair.first);
                pairsVec.emplace_back(innerKeyStr, inner_pair.second);
            }
        }
    }
    return strRanking;
 }


std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, std::vector<int>>>> RankingHandler::getIdxRules(std::string headOrTail) {
    if (!collectRules){
        throw std::runtime_error("The handler option 'collect_rules' is set to false. Recreate the handler with the option set to true.");
    }

    std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, std::vector<Rule*>>>>& data = 
     (headOrTail == "head") ? ranker.getHeadQcandsRules(): ranker.getTailQcandsRules();

    if (!(headOrTail =="head") && !(headOrTail =="tail")){
        throw std::runtime_error("Please specify 'head' or 'tail' as first argument of getRanking");
    }

    std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, std::vector<int>>>> convertedData;

    // relations
    for (const auto &outerPair : data) {
        int outerKey = outerPair.first;
        const auto &middleMap = outerPair.second;

        // source entities
        for (const auto &middlePair : middleMap) {
            int middleKey = middlePair.first;
            const auto &innerMap = middlePair.second;

            // candidates + rules
            for (const auto &innerPair : innerMap) {
                int innerKey = innerPair.first;
                const std::vector<Rule*>& rules = innerPair.second;
                std::vector<int> ruleIds;
                ruleIds.reserve(rules.size());

                for (Rule* rule : rules) {
                    if (rule != nullptr) {
                        ruleIds.push_back(rule->getID());
                    }else{
                        throw std::runtime_error("Rule object does not exist. This should not happen and is an internal error.");
                    }
                }

                convertedData[outerKey][middleKey][innerKey] = ruleIds;
            }
        }
    }
    return convertedData;
}


std::unordered_map<std::string,std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>> RankingHandler::getStrRules(std::string headOrTail) {
    if (!collectRules){
        throw std::runtime_error("The handler option 'collect_rules' is set to false. Recreate the handler with the option set to true.");
    }

    std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, std::vector<Rule*>>>>& data = 
     (headOrTail == "head") ? ranker.getHeadQcandsRules(): ranker.getTailQcandsRules();

    if (!(headOrTail =="head") && !(headOrTail =="tail")){
        throw std::runtime_error("Please specify 'head' or 'tail' as first argument of getRanking");
    }

    std::unordered_map<std::string,std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>> convertedData;

    // relations
    for (const auto &outerPair : data) {
        int outerKey = outerPair.first;
        std::string rel = index->getStringOfRelId(outerKey);
        const auto &middleMap = outerPair.second;

        // source entities
        for (const auto &middlePair : middleMap) {
            int middleKey = middlePair.first;
            std::string source = index->getStringOfNodeId(middleKey);
            const auto &innerMap = middlePair.second;

            // candidates + rules
            for (const auto &innerPair : innerMap) {
                int innerKey = innerPair.first;
                std::string cand = index->getStringOfNodeId(innerKey);
                const std::vector<Rule*>& rules = innerPair.second;
                std::vector<std::string> ruleIds;
                ruleIds.reserve(rules.size());

                for (Rule* rule : rules) {
                    if (rule != nullptr) {
                        ruleIds.push_back(rule->computeRuleString(index.get()));
                    }else{
                        throw std::runtime_error("Rule object does not exist. This should not happen and is an internal error.");
                    }
                }
                convertedData[rel][source][cand] = ruleIds;
            }
        }
    }
    return convertedData;
}





 void RankingHandler::setCollectRules(bool ind){
    collectRules = ind;
 }
