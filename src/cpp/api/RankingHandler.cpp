#include "RankingHandler.h"


#include <string>
#include <map>
#include <functional>


RankingHandler::RankingHandler(std::map<std::string, std::string> options): BackendHandler(){
    setRankingOptions(options, ranker);
}

void RankingHandler::calculateRanking(std::shared_ptr<DataHandler> dHandler){
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
        throw std::runtime_error("Please specify 'head' or 'tail' as last argument of getRanking");
    }
}
