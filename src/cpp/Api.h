#ifndef API_H
#define API_H

#include "core/myClass.h"
#include "core/Index.h"
#include "core/TripleStorage.h"
#include "core/Rule.h"
#include "core/RuleStorage.h"
#include "core/Globals.h"
#include "features/Application.h"


class RuleHandler{
public:
    void calculateRanking();
     RuleHandler(){};
};

#endif