#include "Api.h"


// *** Ranking handler ***
void RuleHandler::calculateRanking(
        std::string targetPath, std::string trainPath, std::string filterPath, std::string rulesPath, std::string write,
        std::map<std::string,std::string> options
    ){

   for(const auto &pair : options){
    std::cout << pair.first << ":" << pair.second << std::endl;
    }
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

std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> RuleHandler::getRanking(std::string headOrTail){
    // std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> newData; 
    // auto& results = (headOrTail=="head") ? ranker.getHeadQcandsConfs() : ranker.getTailQcandsConfs();
    // for (auto& queries: results){
    //     int relation = queries.first;
    //     auto& query = queries.second;
    //     for (auto& cands:query){
    //         int source = cands.first;
    //         newData[relation][source] = cands.second;
    //     }
    // }

    // return newData;



    if (headOrTail=="head"){
        return ranker.getHeadQcandsConfs();
    }else if (headOrTail=="tail"){
        return ranker.getTailQcandsConfs();
    }else{
        throw std::runtime_error("Please specify 'head' or 'tail' as last argument of getRanking");
    }
}

// ***StatsCalculator***

StatsHandler::StatsHandler(std::string dataPath){
    data = std::make_unique<TripleStorage>(index);
    data->read(dataPath);
    storage = std::make_unique<RuleStorage>(index);


}

std::array<int,2> StatsHandler::calcStats(std::string ruleStr){
    std::unique_ptr<Rule> rule = storage->parseAnytimeRule(ruleStr);
}
