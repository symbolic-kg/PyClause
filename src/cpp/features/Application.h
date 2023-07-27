#ifndef RANKING_H
#define RANKING_H

#include <map>

#include "../core/TripleStorage.h"
#include "../core/RuleStorage.h"
#include "../core/Types.h"

struct QueryResults {
    NodeToPredRules candToRules;
    std::vector<std::pair<int, double>> aggrCand;
};

class ApplicationHandler
{
public:
    ApplicationHandler(TripleStorage& target);
    // apply all rules for all queries existig in this->target
    // addFilter can be valid set e.g. , used for additional filtering
    // stores results (cand->Vector<rule*>) in, e.g., this->headQueryResult[rel][source_entity].candToRules
    void calculateHqueryResults(TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter);
    void calculateTqueryResults(TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter);
    // aggregate query results based on _cfg_ defined aggregation function
    // writes to, e.g., this->headQueryResults[rel][source_entitiy].aggrCand
    void aggregateQueryResults();
    //aggregation functions
    void scoreMaxPlus(const NodeToPredRules& candToRules, std::vector<std::pair<int, double>>& aggregatedCand);
private:
    std::unordered_map<int,std::unordered_map<int, QueryResults>> headQueryResults;
    std::unordered_map<int,std::unordered_map<int, QueryResults>> tailQueryResults;
    TripleStorage& target;


};


#endif // RANKING_H