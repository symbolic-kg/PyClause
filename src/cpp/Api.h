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
    RuleHandler(){};
    void calculateRanking(
        std::string targetPath, std::string trainPath, std::string filterPath, std::string rulesPath, std::string write,
        std::map<std::string,std::string> options
    );
    std::unordered_map<int,std::unordered_map<int,std::vector<std::pair<int, double>>>> getRanking(std::string headOrTail);
private:
    ApplicationHandler ranker;
};

#endif