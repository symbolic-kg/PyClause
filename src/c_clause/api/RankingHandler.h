#ifndef RANKING_HANDLER_H
#define RANKING_HANDLER_H

#include "Handler.h"
#include "Loader.h"
#include "../core/Index.h"
#include "../core/TripleStorage.h"
#include "../core/Rule.h"
#include "../core/RuleStorage.h"
#include "../core/RuleFactory.h"
#include "../core/Globals.h"
#include "../features/Application.h"
#include "../core/Index.h"
#include "../core/Util.hpp"
#include "../core/Types.h"



class RankingHandler: public BackendHandler{
public:
    RankingHandler(std::map<std::string, std::string> options);
    void setCollectRules(bool ind);
    // exposed functions
    void writeRanking(std::string writePath, std::shared_ptr<Loader> dHandler);
    void writeRules(std::string writePath, std::shared_ptr<Loader> dHandler, std::string direction, bool strings);
    void calculateRanking(std::shared_ptr<Loader> dHandler);
    std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> getRanking(std::string headOrTail);
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::pair<std::string, double>>>> getStrRanking(std::string headOrTail);
    
   
    //[rel][source][cand] --> vector to rule indices
    // we have relation upfront to efficiently get relation-wise query rankings
    std::unordered_map<int,std::unordered_map<int, std::unordered_map<int, std::vector<int>>>> getIdxRules(std::string headOrTail);
    std::unordered_map<std::string,std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>>> getStrRules(std::string headOrTail);

    void setOptions(std::map<std::string, std::string> options);
    void setOptionsFrontend(std::map<std::string, std::string> options);
private:
    
    ApplicationHandler ranker;
    std::shared_ptr<Index> index;
    std::shared_ptr<Loader> myDhandler;

    //options
    bool collectRules = false;

   

};



#endif