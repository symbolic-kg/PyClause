#ifndef RANKING_H
#define RANKING_H

#include <map>

#include "../core/TripleStorage.h"
#include "../core/RuleStorage.h"
#include "../core/Types.h"



class ApplicationHandler
{
public:
    ApplicationHandler(){};
    // apply all rules for all queries existig in target
    // addFilter can be valid set e.g. , used for additional filtering
    // stores results (cand->Vector<rule*>) in, e.g., this->headQueryResult[rel][source_entity].candToRules
    // direction=head or direction=tail
    // from config filtering with train and addFilter is handable, filtering with target needs to be on triple level
    void calculateQueryResults(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter, std::string direction);
    // aggregate query results based on _cfg_ defined aggregation function
    // writes to, e.g., this->headQueryResults[rel][source_entitiy].aggrCand
    void aggregateQueryResults(std::string direction);
    //aggregation functions
    void scoreMaxPlus(const NodeToPredRules& candToRules, std::vector<std::pair<int, double>>& aggregatedCand);
    // writes to e.g. this->headQueryResults[rel][head].aggrCand
    void makeRanking(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter);
    void writeRanking(TripleStorage& target, std::string path);

    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>>& getHeadQcandsRules();
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>>&  getHeadQcandsConfs();

    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>>&  getTailQcandsRules();
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>>& getTailQcandsConfs();




private:
    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>> headQcandsRules;
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>> headQcandsConfs;

    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>> tailQcandsRules;
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>> tailQcandsConfs;

    // alternative version, prevents storing map twice but data access needs copying

    //struct QueryResults {
    //NodeToPredRules candToRules;
    //std::vector<std::pair<int, double>> aggrCand;
    //};
    //std::unordered_map<int,std::unordered_map<int, QueryResults>> headQueryResults;
    //std::unordered_map<int,std::unordered_map<int, QueryResults>> tailQueryResults;
};


#endif // RANKING_H