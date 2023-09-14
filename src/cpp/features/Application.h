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
    void calculateQueryResults(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter, bool dirIsTail);
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
    void setSaveCandidateRules(bool ind);
    void setPerformAggregation(bool ind);
    void setDiscAtLeast(int num);



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

    // ***ranking options***

    // preselection of candidates based on rules with the highest confidences
    // starting with the rule with higehest conf as soon as numPreselect cands are found we stop
    int rank_numPreselect=1000000;
   
    // filter with train
    bool rank_filterWtrain=true;
    // filter with the target set (mostly the "test" set)
    // note, fitering with additional set e.g. valid is performend by giving filter set as param in ranking functions
    bool rank_filterWtarget=true;
    // aggregation function in {"maxplus"}
    std::string rank_aggrFunc="maxplus";

    // track for each candidate the predicting rules (saved in e.g. headQcandsRules )
    bool saveCandidateRules=false;
    // perform aggregation and store in (headQcandsConfs)
    bool performAggregation=true;

    // num candidates to output in ranking
    // must not be higher than _cfg_rnk_numPreselect;
    int rank_topk=100;

    // number of candidates to at least fully discriminate in a ranking; if this criterion is reached and topk then
    // application is stopped for the current query; atleast: if the first rule predicts more, then all of these have to be discriminated
    int rank_discAtLeast=10;




};


#endif // RANKING_H