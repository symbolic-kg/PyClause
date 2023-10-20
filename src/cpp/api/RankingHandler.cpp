#include "RankingHandler.h"


#include <string>
#include <map>
#include <functional>


RankingHandler::RankingHandler(std::map<std::string, std::string> options): BackendHandler(){
    target = std::make_unique<TripleStorage>(index);
    filter = std::make_unique<TripleStorage>(index);

    // parent constructor is called before
    setRuleOptions(options, *ruleFactory);
    setRankingOptions(options, ranker);
}



void RankingHandler::loadDatasets(std::string targetPath, std::string trainPath, std::string filterPath){
    if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new handler.");
    }

    std::cout<<"Load data... \n";


    target->read(targetPath);
    if (filterPath!=""){
        filter->read(filterPath);
    }
    
    // sets loadedData to true
    loadData(trainPath);
}


void RankingHandler::calculateRanking(){
    ranker.clearAll();
    ranker.makeRanking(*target, *data, *rules, *filter);
}


void RankingHandler::writeRanking(std::string writePath){
    ranker.writeRanking(*target, writePath);

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
