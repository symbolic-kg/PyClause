#ifndef RANKING_HANDLER_H
#define RANKING_HANDLER_H

#include "Api.h"
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
    void loadDatasets(std::string targetPath, std::string trainPath, std::string filterPath);
    void writeRanking(std::string writePath);
    void calculateRanking();
    std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> getRanking(std::string headOrTail);
private:

    /// train is already defined in BackendHandler
    std::unique_ptr<TripleStorage> target;
    std::unique_ptr<TripleStorage> filter;
    ApplicationHandler ranker;
};



#endif