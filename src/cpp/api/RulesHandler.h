#ifndef RULESHANDLER_H
#define RULESHANDLER_H


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
#include "Api.h"
#include "DataHandler.h"
#include "../core/Types.h"

#include <string>
#include <vector>
#include <array>
#include <map>


class RulesHandler: public BackendHandler{
public:
    RulesHandler(std::map<std::string, std::string> options);
    
    void setCollectPredictions(bool ind);
    void setCollectStats(bool ind);
    void setOptions(std::map<std::string, std::string> options);

    // functions exposed to python
    // std::pair<std::vector<std::vector<std::array<std::string, 2>>>, std::vector<std::array<int,2>>>
    void calcRulesPredictions(std::vector<std::string>& stringRules, std::shared_ptr<DataHandler> dHandler);
    std::vector<std::vector<std::array<int, 3>>> getIdxPredictions();
    std::vector<std::vector<std::array<std::string, 3>>> getStrPredictions();
    std::vector<std::array<int,2>>& getStats();


private:
    bool collectPredictions = true;
    bool collectStats = true;
     // for every input rule a collection of triples
    std::vector<std::unordered_set<Triple>> predictions;
    // for every input rule a tuple (num_predictions, num_true_predictions)
    std::vector<std::array<int,2>> stats;

    // we have to cache the input rules when we want to write them to file
    // just string rules no stats in the strings;
    std::vector<std::string> rules; 
    std::shared_ptr<Index> index;
};



#endif