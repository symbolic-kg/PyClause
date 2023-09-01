#include "Api.h"

void RuleHandler::calculateRanking(
        std::string targetPath, std::string trainPath, std::string filterPath, std::string rulesPath, std::string write
        ){
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