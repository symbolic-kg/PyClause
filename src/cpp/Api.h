#ifndef API_H
#define API_H

#include "core/myClass.h"
#include "core/Index.h"
#include "core/TripleStorage.h"
#include "core/Rule.h"
#include "core/RuleStorage.h"
#include "core/RuleFactory.h"
#include "core/Globals.h"
#include "features/Application.h"
#include "core/Index.h"
#include "core/Util.hpp"
#include "core/Types.h"

#include <array>
#include <string>


class RankingHandler{
public:
    RankingHandler(){};
    void calculateRanking(
        std::string targetPath, std::string trainPath, std::string filterPath, std::string rulesPath, std::string write,
        std::map<std::string,std::string> options = std::map<std::string, std::string>()
    );
    std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> getRanking(std::string headOrTail);
private:
    ApplicationHandler ranker;
    // setable options in frontend for ranking are registered here
    void setRankingOptions(std::map<std::string,std::string> options);
};


class RuleHandler{
public:
    RuleHandler(std::string dataPath);
    std::array<int,2> calcStats(std::string ruleStr);
    std::pair<std::vector<std::vector<std::array<std::string, 2>>>, std::vector<std::array<int,2>>> calcRulesPredictions(std::vector<std::string> stringRules,  bool retPredictions, bool retStats);
private:
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::unique_ptr<TripleStorage> data;
    std::unique_ptr<RuleFactory> ruleFactory;
};

#endif