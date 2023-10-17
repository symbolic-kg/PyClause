#ifndef API_H
#define API_H

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

#include <array>
#include <string>


class BackendHandler{
public:
    BackendHandler();
    // extend to overloaded functions: load data from a list of string triples/ list of indice triples
    void loadData(std::string path);
    void loadRules(std::string path);
private:
protected:
    std::shared_ptr<Index> index;
    // data aka train where rule application is performed
    std::unique_ptr<TripleStorage> data;
    std::shared_ptr<RuleFactory> ruleFactory;
    std::unique_ptr<RuleStorage> rules;

    // handling
    bool loadedData = false;

    

};


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
    // setable options for individual rule classes (assigned to RuleFactory)
    void setRuleOptions(std::map<std::string,std::string> options, RuleFactory& ruleFact);
};


class RuleHandler{
public:
    RuleHandler(std::string dataPath);
    std::pair<std::vector<std::vector<std::array<std::string, 2>>>, std::vector<std::array<int,2>>> calcRulesPredictions(std::vector<std::string> stringRules,  bool retPredictions, bool retStats);
private:
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::unique_ptr<TripleStorage> data;
    std::unique_ptr<RuleFactory> ruleFactory;
};

#endif