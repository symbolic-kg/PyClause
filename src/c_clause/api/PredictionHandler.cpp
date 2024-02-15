#include <functional>
#include <fstream>

#include "PredictionHandler.h"
#include "../core/Types.h"

PredictionHandler::PredictionHandler(std::map<std::string, std::string> options){
    auto verb = options.find("verbose");
    if (verb!=options.end()){
        this->verbose = util::stringToBool(verb->second);
    }
    setOptions(options, scorer);    
}

void PredictionHandler::setOptionsFrontend(std::map<std::string, std::string> options){
    setOptions(options, scorer); 
}

void PredictionHandler::setOptions(std::map<std::string, std::string> options, ApplicationHandler& scorer){

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

void PredictionHandler::scoreTriples(std::vector<std::array<int, 3>> triples,  std::shared_ptr<Loader> dHandler){
    if (!dHandler->getLoadedData() || !dHandler->getLoadedRules()){
        throw std::runtime_error("You must first load data and load rules with the loader before scoring triples.");
    }
    scorer.clearAll();
    index = dHandler->getIndex();
    scorer.calculateTripleScores(triples, dHandler->getData(), dHandler->getRules());
}


void PredictionHandler::scoreTriples(std::vector<std::array<std::string, 3>> triples,  std::shared_ptr<Loader> dHandler){
    if (!dHandler->getLoadedData() || !dHandler->getLoadedRules()){
        throw std::runtime_error("You must first load data and load rules with the loader before scoring triples.");
    }

    if (scorer.getScoreCollectGroundings()){
        // safety measure to bind the leftime of the rules (stored in RulesHandler of Loader) to the lifetime of this object
        myDHandler = dHandler;
    }

    scorer.clearAll();
    std::vector<Triple> idxTriples(triples.size());
    index = dHandler->getIndex();
    for (int i=0; i<triples.size(); i++){
       try {
            int head = index->getIdOfNodestring(triples[i][0]);
            int rel = index->getIdOfRelationstring(triples[i][1]);
            int tail = index->getIdOfNodestring(triples[i][2]);
            idxTriples.at(i) = {head, rel , tail};
       } catch(const std::exception& e){
            throw std::runtime_error(
                "An entity or relation in a triple is not known, i.e., not loaded with the data. "
                "You can only calculate scores for triples where all elements are known: "
                + triples[i][0] + " " + triples[i][1] +  " " + triples[i][2]
            );
       }

    }
    scorer.calculateTripleScores(idxTriples, dHandler->getData(), dHandler->getRules());
}


void PredictionHandler::scoreTriples(std::string path,  std::shared_ptr<Loader> dHandler){
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
        int tail = (int) (arr[2]);
        out.at(it) = {index->getStringOfNodeId(head), index->getStringOfRelId(rel), index->getStringOfNodeId(tail), std::to_string(arr[3])};
        it++;
    }
    return out;
}


void PredictionHandler::writeScores(std::string& path, bool asString){
    std::vector<std::array<double, 4>>& scores = scorer.getTripleScores();

    std::ofstream file(path);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + path);
    }

    for (int i=0; i<scores.size(); i++){

 

        int ihead = static_cast<int>(scores[i][0]);
        int irel = static_cast<int>(scores[i][1]);
        int itail = static_cast<int>(scores[i][2]);

        std::string head = asString ? index->getStringOfNodeId(ihead) : std::to_string(ihead);
        std::string rel = asString ? index->getStringOfRelId(irel)   : std::to_string(irel);
        std::string tail = asString ? index->getStringOfNodeId(itail) : std::to_string(itail);
        std::string score = std::to_string(scores[i][3]);

        file << head + "\t" + rel + "\t" + tail + "\t" + score;

        if (i<scores.size()-1){
            file<<"\n";
        }
    }
}


std::tuple<std::vector<std::array<std::string,3>>, std::vector<std::vector<std::string>>,  std::vector<std::vector<std::vector<std::vector<std::array<std::string,3>>>>>> PredictionHandler::getStrExplanations(){
    if (!scorer.getScoreCollectGroundings()){
        throw std::runtime_error(
            "You have set 'prediction_handler.collect_explanation=False. Please set the option to true when you want to output explanations"
        );
    }
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
            strRulesPerTarget.push_back(rule->computeRuleString(index.get()));
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
    if (!scorer.getScoreCollectGroundings()){
        throw std::runtime_error(
            "You have set 'prediction_handler.collect_explanation=False. Please set the option to true when you want to output explanations"
        );
    }

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

void PredictionHandler::writeExplanations(std::string& outputPath, bool asString){
    if (!scorer.getScoreCollectGroundings()){
        throw std::runtime_error(
            "You have set 'prediction_handler.collect_explanation=False. Please set the option to true when you want to output explanations"
        );
    }

    std::ofstream file(outputPath);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + outputPath);
    }

    std::vector<std::pair<Triple, RuleGroundings>>& trGroundings = scorer.getTripleGroundings();


    // for each target
    for (int i=0; i<trGroundings.size(); i++){
        file << "{";
        std::pair<Triple, RuleGroundings>& el = trGroundings[i];
        Triple triple = el.first;


        std::string head = asString ? "\"" + index->getStringOfNodeId(triple[0]) + "\"" : std::to_string(triple[0]);
        std::string rel = asString ? "\"" + index->getStringOfRelId(triple[1]) + "\"" : std::to_string(triple[1]);
        std::string tail = asString ? "\"" + index->getStringOfNodeId(triple[2]) + "\"" : std::to_string(triple[2]);

        file <<"\"target\":";
        file<<"[" + head + "," + rel + "," + tail + "]" + ",";


        RuleGroundings& elGroundings = el.second;
        std::vector<Rule*> rulesPerTarget;
        std::vector<std::vector<std::vector<std::array<int, 3>>>> expPerTarget;
        // for every rule + groundings that predicted the target
        for (auto& pair: elGroundings){
            Rule* rule = pair.first;
            rulesPerTarget.push_back(rule);
            // groundings for the rule
            std::vector<std::vector<Triple>>& explanations = pair.second;
            expPerTarget.push_back(explanations);
        }

        file<<"\"rules\":[";
        // rules
        for (int j=0; j<rulesPerTarget.size(); j++){
            if (asString){
                file<< "\"" + rulesPerTarget[j]->computeRuleString(index.get()) + "\"";
            } else {
                file<< "\"" + std::to_string(rulesPerTarget[j]->getID()) + "\"";
            }

            if (j<rulesPerTarget.size()-1){
                file<<",";
            }
        }

        file<<"],";

        // groundings
        file<<"\"groundings\":[";
        for (int j=0; j<rulesPerTarget.size(); j++){
          file<<groundingsToString(expPerTarget[j], asString);
          if (j<rulesPerTarget.size()-1){
            file<<",";
          }
        }
        file<<"]";

        file<<"}";

        if (i<trGroundings.size()-1){
            file<<"\n";
        }
    }
    file.close();
}

// groundings for one rule: list of groundings; where a grounding is a list of triples
std::string PredictionHandler::groundingsToString(std::vector<std::vector<Triple>> groundings, bool asString){
     std::string json = "["; 

    for (size_t i = 0; i < groundings.size(); ++i) {
        json += "[";

        for (size_t j = 0; j < groundings[i].size(); ++j) {
            std::string head = asString ? "\"" + index->getStringOfNodeId(groundings[i][j][0]) + "\"" : std::to_string(groundings[i][j][0]);
            std::string rel = asString ? "\"" + index->getStringOfRelId(groundings[i][j][1]) + "\"" : std::to_string(groundings[i][j][1]);
            std::string tail = asString ? "\"" + index->getStringOfNodeId(groundings[i][j][2]) + "\"" : std::to_string(groundings[i][j][2]);

            json += "[" + head + "," + rel + "," + tail + "]";

            if (j < groundings[i].size() - 1) {
                json += ", "; // separate triples of one grounding with a comma
            }
        }
        json += "]"; // end of one grounding

        if (i < groundings.size() - 1) {
            json += ", "; // grounding separation
        }
    }
    json += "]"; 
    return json;
}





