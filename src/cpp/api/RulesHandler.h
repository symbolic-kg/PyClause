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

#include <string>
#include <vector>
#include <array>


class RulesHandler: public BackendHandler{
public:
    RulesHandler(): BackendHandler(){};
    std::pair<std::vector<std::vector<std::array<std::string, 2>>>, std::vector<std::array<int,2>>> calcRulesPredictions(std::vector<std::string> stringRules, bool retPredictions, bool retStats);


};



#endif