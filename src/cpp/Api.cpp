#include "Api.h"
#include "functional"


// *** Ranking handler ***
void RankingHandler::calculateRanking(
        std::string targetPath, std::string trainPath, std::string filterPath, std::string rulesPath, std::string write,
        std::map<std::string,std::string> options
    ){

    setRankingOptions(options);

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

    std::cout<<"data loaded. \n";


    RuleStorage rules(index);
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



// ***StatsCalculator***

RuleHandler::RuleHandler(std::string dataPath){
    data = std::make_unique<TripleStorage>(index);
    data->read(dataPath);
    storage = std::make_unique<RuleStorage>(index);


}

std::array<int,2> RuleHandler::calcStats(std::string ruleStr){
    std::unique_ptr<Rule> rule = storage->parseAnytimeRule(ruleStr);
    // reset the exact stats, actually not needed as rule was just created
    rule->setStats(0, 0, true);
    rule->setTrackInMaterialize(true);
    //you could collect the data here
    rule->materialize(*data);
    return rule->getStats(true);   
}
