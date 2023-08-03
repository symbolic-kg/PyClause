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
    auto& data = (headOrTail=="head") ? ranker.getHeadQueryResults() : ranker.getTailQueryResults();
    // TODO this is an unfortunate copy operation as serialization will produce another copy
    // in the applicationHandler could instead have two maps where one stores aggregated results
    // and one stores candidateRuleResults, that means, however, more copying whenever rankings are created
    std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> newData;
    for(const auto& pair1 : data) {
        auto& submap = pair1.second;
        for(const auto& pair2 : submap) {
            newData[pair1.first][pair2.first] = pair2.second.aggrCand;
        }
    }
    return newData;
}