#include "RulesHandler.h"

#include <functional>

RulesHandler::RulesHandler(std::map<std::string, std::string> options): BackendHandler(){
    setOptions(options);
}


void RulesHandler::setOptions(std::map<std::string, std::string> options){


    // register options for ranker

     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        {"collect_predictions", [this](std::string val) {this->setCollectPredictions(util::stringToBool(val));}},
        {"collect_statistics", [this](std::string val) {this->setCollectStats(util::stringToBool(val));}},
        {"num_threads", [this](std::string val) {this->setNumThr(std::stoi(val));}}
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


void RulesHandler::setOptionsFrontend(std::map<std::string, std::string> options){
    setOptions(options);
}


void RulesHandler::setCollectPredictions(bool ind){
    collectPredictions = ind;
}


void RulesHandler::setCollectStats(bool ind){
    collectStats = ind;
}

void RulesHandler::setNumThr(int num){
    if (num==-1){
        num_thr = omp_get_max_threads();
    }else{
        num_thr = num;
    }
}


void RulesHandler::calcRulesPredictions(std::vector<std::string>& stringRules, std::shared_ptr<Loader> dHandler){

    index = dHandler->getIndex();
    predictions.clear();
    stats.clear();
    rules.clear();

    if (!dHandler->getLoadedData()){
        throw std::runtime_error("Please load data before you calculate rule predictions/stats.");
    }

    if (!ruleFactory){
        // use your own factory to not interfere with the options of the data loader
        ruleFactory = std::make_unique<RuleFactory>(dHandler->getIndex());
        ruleFactory->setCreateRuleB(true);
        ruleFactory->setCreateRuleC(true);
        ruleFactory->setCreateRuleD(true);
        ruleFactory->setCreateRuleXXc(true);
        ruleFactory->setCreateRuleXXd(true);
        ruleFactory->setCreateRuleZ(true);
    }
    

    rules.resize(stringRules.size());


    if (collectPredictions){
        predictions.resize(stringRules.size());
    }

    if (collectStats){
        stats.resize(stringRules.size());
    }

    #pragma omp parallel num_threads(num_thr)
    {
        TripleStorage& data = dHandler->getData();
        std::shared_ptr<Index> index = dHandler->getIndex();

        #pragma omp for schedule(dynamic)
        for (int i=0; i<stringRules.size(); i++){
            std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(stringRules[i]);
            rule->setTrackInMaterialize(collectStats);
            if (!rule){
                throw std::runtime_error("Error in parsing rule:" + stringRules[i]);
            }
            std::unordered_set<Triple> outputs;
            rule->materialize(data, outputs);
            #pragma omp critical
            {   
                if (collectPredictions){
                    predictions.at(i) = outputs;
                    outputs.clear();
                }

                if (collectStats){
                    stats.at(i) = rule->getStats(true);
                }    
            } 
        }
    }
}


std::vector<std::vector<std::array<int, 3>>> RulesHandler::getIdxPredictions(){
    if (!collectPredictions){
        throw std::runtime_error(
            "The handler has set rules_handler.collect_predictions=False. Please set the option to true before creating the handler."
            );
    }


    std::vector<std::vector<std::array<int, 3>>> out(predictions.size());
    for (int i=0; i<predictions.size(); i++){
        out[i] = std::vector<std::array<int,3>>(predictions[i].begin(), predictions[i].end());
    }
    return out;
}


std::vector<std::vector<std::array<std::string, 3>>> RulesHandler::getStrPredictions(){

     if (!collectPredictions){
        throw std::runtime_error(
            "The handler has set rules_handler.collect_predictions=False. Please set the option to true before creating the handler."
            );
    }

    std::vector<std::vector<std::array<std::string, 3>>> out(predictions.size());

    for (int i=0; i<predictions.size(); i++){
        for (auto& triple: predictions[i]){
            std::string head = index->getStringOfNodeId(triple[0]);
            std::string rel = index->getStringOfRelId(triple[1]);
            std::string tail = index->getStringOfNodeId(triple[2]);
            out[i].push_back({head, rel, tail});
        }
    }
    return out;
}

std::vector<std::array<int,2>>& RulesHandler::getStats(){
     if (!collectStats){
        throw std::runtime_error(
            "The handler has set rules_handler.collect_statistics=False. Please set the option to true before creating the handler."
            );
    }
    return stats;
}