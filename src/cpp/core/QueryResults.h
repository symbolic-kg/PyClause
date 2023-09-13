#ifndef QUERYRESULTS_H
#define QUERYRESULTS_H

#include "Types.h"

#include <vector>




//temporary result handling for one query
class QueryResults{
public:
    QueryResults(int addTopK);
    //updates if key exists, adds if it doesnt
    void insertRule(int cand, Rule* rule);
    int size();
    bool contains(int cand);
    bool empty();
    //delete all data
    void clear();
    std::vector<Rule*>& getRulesForCand(int cand);
    NodeToPredRules& getCandRules();
    


private:
    NodeToPredRules candRules;
    std::vector<int> candidateOrder;

    // maximal number of candidates to add
    int addTopK;

   



};


#endif
