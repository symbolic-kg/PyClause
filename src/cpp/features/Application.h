#ifndef RANKING_H
#define RANKING_H

#include <map>
#include <unordered_map>
#include <omp.h>

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
    void aggregateQueryResults(std::string direction, TripleStorage& train);
    //aggregation functions
    void scoreMaxPlus(const NodeToPredRules& candToRules, std::vector<std::pair<int, double>>& aggregatedCand, TripleStorage& train);
    // writes to e.g. this->headQueryResults[rel][head].aggrCand
    void makeRanking(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter);
    void writeRanking(TripleStorage& target, std::string path);

    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>>& getHeadQcandsRules();
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>>&  getHeadQcandsConfs();

    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>>&  getTailQcandsRules();
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>>& getTailQcandsConfs();
    void clearAll();

    void setNumPreselect(int num);
    void setTopK(int topk);
    void setFilterWTrain(bool ind);
    void setFilterWtarget(bool ind);
    void setAggregationFunc(std::string func);
    void setSaveCandidateRules(bool ind);
    void setPerformAggregation(bool ind);
    void setDiscAtLeast(int num);
    void setTieHandling(std::string opt);
    void setVerbose(bool ind);
    void setNumThr(int num);
    // scoring
    void setScoreNumTopRules(int num);
    void setScoreCollectGroundings(bool ind);
    bool getScoreCollectGroundings();

    //triple scoring
    void calculateTripleScores(std::vector<Triple> triples, TripleStorage& train, RuleStorage& rules);
    // each element is head,rel,tail,score
    std::vector<std::array<double, 4>>& getTripleScores();
    // typedef std::unordered_map<Rule*, std::vector<std::vector<Triple>>> RuleGroundings;
    std::vector<std::pair<Triple, RuleGroundings>>& getTripleGroundings();



private:
    // tail candidates for head queries and all the respective rules that predicted them
    // [relation][tail] --> cands
    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>> headQcandsRules;
    // tail candidates and the aggregated confidences
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>> headQcandsConfs;

    // se above vice versa
    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>> tailQcandsRules;
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>> tailQcandsConfs;

    // alternative version, prevents storing map twice but data access needs copying

    //struct QueryResults {
    //NodeToPredRules candToRules;
    //std::vector<std::pair<int, double>> aggrCand;
    //};
    //std::unordered_map<int,std::unordered_map<int, QueryResults>> headQueryResults;
    //std::unordered_map<int,std::unordered_map<int, QueryResults>> tailQueryResults;

    void sortAndProcessNoisy(std::vector<std::pair<int,double>>& candScoresToSort, QueryResults& qResults, TripleStorage& data);
    void sortAndProcessMax(std::vector<std::pair<int,double>>& candScoresToSort, QueryResults& qResults, TripleStorage& data);
    


    // tripleScores the first 3 elements is the triple head,rel,tail the last element is the score
    // data type is chosen such that scores can be outputted fast
    std::vector<std::array<double, 4>> tripleScores;
    std::vector<std::pair<Triple, RuleGroundings>> tripleGroundings;


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

    // how to handle ties when 2 candidates cannot be further discriminated by rules
    // in {random, frequency}
    // random: randomly rank (expect difference results); frequency: rank according to entity frequency (determinstic results)
    // note that this parameter is independent of any evaluation; it's on the model side
    std::string rank_tie_handling="frequency";

    //***running options***
    // output current relation and direction during ranking
    bool verbose = true;
    // set to -1 for all possible threads
    int num_thr = omp_get_max_threads();


    //***triple scoring options***

    // after how many of the top (conf) rules to stop scoring
    // if maxplus aggregation is chosen this is set to 1
    // might be changed in the future, currently maxplus outputs the max scores
    // ALSO USED for ranking
    int score_numTopRules = -1;

    // for each scores triples the rules and groundings of the rules are collected
    // how many rules is directly affected by score_numTopRules
    bool score_collectGr=false;

};


#endif // RANKING_H