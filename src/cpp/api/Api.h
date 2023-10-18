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
    bool loadedRules = false;

    void setRuleOptions(std::map<std::string, std::string> options, RuleFactory& ruleFactory);
    void setRankingOptions(std::map<std::string, std::string> options, ApplicationHandler& ranker);

    //general 
    bool verbose = true;    

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