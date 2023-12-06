#include "RankingHandler.h"


#include <string>
#include <map>
#include <functional>


RankingHandler::RankingHandler(std::map<std::string, std::string> options): BackendHandler(){
    setRankingOptions(options, ranker);
}

void RankingHandler::calculateRanking(std::shared_ptr<DataHandler> dHandler){
    index = dHandler->getIndex();
    ranker.clearAll();
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
