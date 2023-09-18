#include "Api.h"

#include <functional>
#include <array>
#include <chrono>


// *** Ranking handler ***
void RankingHandler::calculateRanking(
        std::string targetPath, std::string trainPath, std::string filterPath, std::string rulesPath, std::string write,
        std::map<std::string,std::string> options
    ){

    setRankingOptions(options);
    std::cout<<"Load data... \n";

    // data loading
    std::shared_ptr<Index> index = std::make_shared<Index>();
    TripleStorage data(index);
    data.read(trainPath);
   
    //test
    TripleStorage test(index);
    test.read(targetPath);

    //valid 
    TripleStorage valid(index);
    valid.read(filterPath);

    std::cout<<"Data loaded. \n";
    std::cout<<"Loading rules.... \n";


    std::shared_ptr<RuleFactory> ruleFactory = std::make_shared<RuleFactory>(index);
    RuleStorage rules(index, ruleFactory);
    rules.readAnyTimeFormat(rulesPath, true);
    ranker.makeRanking(test, data, rules, valid);

    if (!write.empty()){
        ranker.writeRanking(test, write);
    }
}

std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> RankingHandler::getRanking(std::string headOrTail){
    if (headOrTail=="head"){
        return ranker.getHeadQcandsConfs();
    }else if (headOrTail=="tail"){
        return ranker.getTailQcandsConfs();
    }else{
        throw std::runtime_error("Please specify 'head' or 'tail' as last argument of getRanking");
    }
}

void RankingHandler::setRankingOptions(std::map<std::string, std::string> options){
    

    // register options for ranker

     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        {"topk", [this](std::string val) { ranker.setTopK(std::stoi(val)); }},
        {"num_preselect", [this](std::string val) { ranker.setNumPreselect(std::stoi(val)); }},
        {"aggregation_function", [this](std::string val) { ranker.setAggregationFunc(val); }},
        {"filter_w_train", [this](std::string val) { ranker.setFilterWTrain(util::stringToBool(val)); }},
        {"filter_w_target", [this](std::string val) { ranker.setFilterWtarget(util::stringToBool(val)); }},
        {"disc_at_least", [this](std::string val) { ranker.setDiscAtLeast(std::stoi(val)); }}
    };

    for (auto& handler : handlers) {
        auto opt = options.find(handler.name);
        if (opt != options.end()) {
            if (_cfg_verbose){
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

std::array<int,2> RuleHandler::calcStats(std::string ruleStr){
    std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(ruleStr);
    // reset the exact stats, actually not needed as rule was just created
    rule->setStats(0, 0, true);
    rule->setTrackInMaterialize(true);
    //you could collect the data here
    rule->materialize(*data);
    return rule->getStats(true);   
}

std::pair<std::vector<std::vector<std::array<std::string, 2>>>, std::vector<std::array<int,2>>> RuleHandler::calcRulesPredictions(std::vector<std::string> stringRules, bool retPredictions, bool retStats){
    //TODO: remove timing
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<std::array<std::string,2>>> preds;
    std::vector<std::array<int,2>> stats;


    if (retPredictions){
         preds.resize(stringRules.size());
    }

    #pragma omp parallel
    {
        #pragma omp for
        for (int i=0; i<stringRules.size(); i++){
            std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(stringRules[i]);
            rule->setTrackInMaterialize(retStats);
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
                     stats.push_back(rule->getStats(true));
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


