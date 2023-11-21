#include <functional>

#include "PredictionHandler.h"
#include "../core/Types.h"

PredictionHandler::PredictionHandler(std::map<std::string, std::string> options){

    setOptions(options, scorer);    

}

void PredictionHandler::setOptions(std::map<std::string, std::string> options, ApplicationHandler& scorer){

    // register options for ranker

     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        {"aggregation_function", [&scorer](std::string val) {scorer.setAggregationFunc(val);}},
        {"collect_explanations", [&scorer](std::string val) {scorer.setScoreCollectGroundings(util::stringToBool(val));}},
        {"num_top_rules", [&scorer](std::string val) {scorer.setScoreNumTopRules(std::stoi(val));}},
        {"num_threads", [&scorer](std::string val) {scorer.setNumThr(std::stoi(val));}},


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

void PredictionHandler::scoreTriples(std::vector<std::array<int, 3>> triples,  std::shared_ptr<DataHandler> dHandler){
    if (!dHandler->getLoadedData() || !dHandler->getLoadedRules()){
        throw std::runtime_error("You must first load data and load rules with the loader before scoring triples.");
    }
    scorer.clearAll();
    index = dHandler->getIndex();
    scorer.calculateTripleScores(triples, dHandler->getData(), dHandler->getRules());
}


void PredictionHandler::scoreTriples(std::vector<std::array<std::string, 3>> triples,  std::shared_ptr<DataHandler> dHandler){
    if (!dHandler->getLoadedData() || !dHandler->getLoadedRules()){
        throw std::runtime_error("You must first load data and load rules with the loader before scoring triples.");
    }

    scorer.clearAll();
    std::vector<Triple> idxTriples(triples.size());
    index = dHandler->getIndex();
    for (int i=0; i<triples.size(); i++){
       int head = index->getIdOfNodestring(triples[i][0]);
       int rel = index->getIdOfRelationstring(triples[i][1]);
       int tail = index->getIdOfNodestring(triples[i][2]);
       idxTriples.at(i) = {head, rel , tail};
    }
    scorer.calculateTripleScores(idxTriples, dHandler->getData(), dHandler->getRules());
}


void PredictionHandler::scoreTriples(std::string path,  std::shared_ptr<DataHandler> dHandler){
    std::unique_ptr<std::vector<Triple>> triples;
    triples = dHandler->loadTriplesToVec(path);
    scorer.clearAll();
    index = dHandler->getIndex();
    scorer.calculateTripleScores(*triples, dHandler->getData(), dHandler->getRules());
}

std::vector<std::array<double, 4 >> PredictionHandler::getIdxScores(){
    return scorer.getTripleScores();
}

std::vector<std::array<std::string, 4>> PredictionHandler::getStrScores(){
    std::vector<std::array<double, 4>>& scores = scorer.getTripleScores();
    std::vector<std::array<std::string, 4>> out(scores.size());
    int it = 0;
    for (std::array<double, 4>& arr: scores){
        std::array<std::string, 3> triple;
        int head = (int) arr[0];
        int rel = (int) arr[1];
        int tail = int (arr[2]);
        out.at(it) = {index->getStringOfNodeId(head), index->getStringOfRelId(rel), index->getStringOfNodeId(tail), std::to_string(arr[3])};
        it++;
    }
    return out;
}

std::tuple<std::vector<std::array<std::string,3>>, std::vector<std::vector<std::string>>,  std::vector<std::vector<std::vector<std::vector<std::array<std::string,3>>>>>> PredictionHandler::getStrExplanations(){
    std::vector<std::array<std::string, 3>> targets;
    std::vector<std::vector<std::string>> strRules;
    std::vector<std::vector<std::vector<std::vector<std::array<std::string,3>>>>> groundings;


    std::vector<std::pair<Triple, RuleGroundings>>& trGroundings = scorer.getTripleGroundings();


    // for each target
    for (int i=0; i<trGroundings.size(); i++){
        std::pair<Triple, RuleGroundings>& el = trGroundings[i];
        Triple triple = el.first;
        targets.push_back({index->getStringOfNodeId(triple[0]), index->getStringOfRelId(triple[1]), index->getStringOfNodeId(triple[2])});
        RuleGroundings& elGroundings = el.second;
        std::vector<std::string> strRulesPerTarget;
        std::vector<std::vector<std::vector<std::array<std::string, 3>>>> expPerTarget;
        // for every rule + groundings that predicted the target
        for (auto& pair: elGroundings){
            Rule* rule = pair.first;
            strRulesPerTarget.push_back(rule->getRuleString());
            // groundings for the rule
            std::vector<std::vector<Triple>>& explanations = pair.second;
            std::vector<std::vector<std::array<std::string, 3>>> strExplanations;
            // for every one grounding; aka a sequence of triples
            for (std::vector<Triple>& ruleExp: explanations){
                std::vector<std::array<std::string, 3>> oneStrGrounding;
                for (Triple& tri : ruleExp){
                    std::string strGrhead = index->getStringOfNodeId(tri[0]);
                    std::string strGrRel = index->getStringOfRelId(tri[1]);
                    std::string strGrTail = index->getStringOfNodeId(tri[2]);
                    oneStrGrounding.push_back({strGrhead, strGrRel, strGrTail});
                }
                strExplanations.push_back(oneStrGrounding);
            }
            expPerTarget.push_back(strExplanations);
        }
        groundings.push_back(expPerTarget);
        strRules.push_back(strRulesPerTarget);

    }
    return std::make_tuple(targets, strRules, groundings);
}


std::tuple<std::vector<std::array<int,3>>, std::vector<std::vector<int>>,  std::vector<std::vector<std::vector<std::vector<std::array<int,3>>>>>> PredictionHandler::getIdxExplanations(){
    std::vector<std::array<int, 3>> targets;
    std::vector<std::vector<int>> rulesIdxs;
    std::vector<std::vector<std::vector<std::vector<std::array<int,3>>>>> groundings;


    std::vector<std::pair<Triple, RuleGroundings>>& trGroundings = scorer.getTripleGroundings();


    // for each target
    for (int i=0; i<trGroundings.size(); i++){
        std::pair<Triple, RuleGroundings>& el = trGroundings[i];
        Triple triple = el.first;
        targets.push_back({triple[0], triple[1], triple[2]});
        RuleGroundings& elGroundings = el.second;
        std::vector<int> rulesPerTarget;
        std::vector<std::vector<std::vector<std::array<int, 3>>>> expPerTarget;
        // for every rule + groundings that predicted the target
        for (auto& pair: elGroundings){
            Rule* rule = pair.first;
            rulesPerTarget.push_back(rule->getID());
            // groundings for the rule
            std::vector<std::vector<Triple>>& explanations = pair.second;
            expPerTarget.push_back(explanations);
        }
        groundings.push_back(expPerTarget);
        rulesIdxs.push_back(rulesPerTarget);
    }
    return std::make_tuple(targets, rulesIdxs, groundings);
}





