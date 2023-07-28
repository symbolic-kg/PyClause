#include "Api.h"


void RuleHandler::calculateRanking(){
    // data loading
    std::shared_ptr<Index> index = std::make_shared<Index>();
    TripleStorage data(index);
    data.read(_cfg_pathTrain);
   
    //test
    TripleStorage test(index);
    test.read(_cfg_pathTarget);

    //valid 
    TripleStorage valid(index);
    valid.read(_cfg_pathFilter);

    std::cout<<"data loaded. \n";


    RuleStorage rules(index);
    rules.readAnyTimeFormat(_cfg_pathRules, true);

    ApplicationHandler ranker;
    ranker.makeRanking(test, data, rules, valid);
    ranker.writeRanking(test, _cfg_pathRankFile);
}