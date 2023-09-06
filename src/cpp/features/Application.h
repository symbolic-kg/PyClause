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

    void setNumPreselect(int num);
    void setTopK(int topk);
    void setFilterWTrain(bool ind);
    void setFilterWtarget(bool ind);
    void setAggregationFunc(std::string func);



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

    // ***ranking parameters***
    // preselection of candidates based on rules with the highest confidences
    // for noisy-or this will result in slightly incorrect rankings when set low
    // for max-plus ranking is correct
    int rank_numPreselect=100;
    // num candidates to output in ranking
    // must not be higher than _cfg_rnk_numPreselect;
    int rank_topk=100;
    // filter with train
    bool rank_filterWtrain=true;
    // filter with the target set (mostly the "test" set)
    // note, fitering with additional set e.g. valid is performend by giving filter set as param in ranking functions
    bool rank_filterWtarget=true;
    // aggregation function in {"maxplus"}
    std::string rank_aggrFunc="maxplus";



};


#endif // RANKING_H