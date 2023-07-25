#ifndef RANKING_H
#define RANKING_H

#include <map>

#include "../core/TripleStorage.h"
#include "../core/RuleStorage.h"
#include "../core/Types.h"

class RankingHandler
{
public:
    // filter can be valid set e.g. , used for additional filtering
    void calculateQueryResults(TripleStorage& train, TripleStorage& target, RuleStorage& rules, TripleStorage& addFilter);
    void scoreMaxPlus(NodeToPredRules queryResult, std::map<int,double> scoredResult);
private:
    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>> headQueryResults;
    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>> tailQueryResults;



};


#endif // RANKING_H