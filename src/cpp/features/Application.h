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
    ApplicationHandler(){};
    // apply all rules for all queries existig in this->target
    // addFilter can be valid set e.g. , used for additional filtering
    // stores results (cand->Vector<rule*>) in, e.g., this->headQueryResult[rel][source_entity].candToRules
    // direction=head or direction=tail
    void calculateQueryResults(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter, std::string direction);
    // aggregate query results based on _cfg_ defined aggregation function
    // writes to, e.g., this->headQueryResults[rel][source_entitiy].aggrCand
    void aggregateQueryResults(std::string direction);
    //aggregation functions
    void scoreMaxPlus(const NodeToPredRules& candToRules, std::vector<std::pair<int, double>>& aggregatedCand);
    // writes to e.g. this->headQueryResults[rel][head].aggrCand
    void makeRanking(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter);
    void writeRanking(TripleStorage& target, std::string path);


private:
    std::unordered_map<int,std::unordered_map<int, QueryResults>> headQueryResults;
    std::unordered_map<int,std::unordered_map<int, QueryResults>> tailQueryResults;
};


#endif // RANKING_H