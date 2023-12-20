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

// all API handler classes inherit from this base class
class BackendHandler{
public:
    BackendHandler();
    virtual void setOptionsFrontend(std::map<std::string, std::string> options) = 0;

private:
protected:
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