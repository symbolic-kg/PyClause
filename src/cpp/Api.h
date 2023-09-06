#ifndef API_H
#define API_H

#include "core/myClass.h"
#include "core/Index.h"
#include "core/TripleStorage.h"
#include "core/Rule.h"
#include "core/RuleStorage.h"
#include "core/Globals.h"
#include "features/Application.h"
#include "core/Index.h"

#include <array>
#include <string>


class RuleHandler{
public:
    RuleHandler(){};
    void calculateRanking(
        std::string targetPath, std::string trainPath, std::string filterPath, std::string rulesPath, std::string write,
        std::map<std::string,std::string> options
    );
    std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> getRanking(std::string headOrTail);
private:
    ApplicationHandler ranker;
};


class StatsHandler{
public:
    StatsHandler(std::string dataPath);
    std::array<int,2> calcStats(std::string ruleStr);
private:
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::unique_ptr<TripleStorage> data;
    std::unique_ptr<RuleStorage> storage;
};

#endif