#ifndef QUERYRESULTS_H
#define QUERYRESULTS_H

#include "Types.h"

#include <vector>
#include <string>



class Rule;

//temporary result handling for one query
// note that this structure assumes throughout that candiates are added in order of the highest rule confidences
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
    // checks if at least discAtLeast top candidates can be fully disciminated
    bool checkDiscrimination();
    


private:
    NodeToPredRules candRules;
    std::vector<int> candidateOrder;

    // maximal number of candidates to add
    int addTopK;

    // the num of top candidates which at least have to be fully disciminated
    int discAtLeast = 10;
    // we track at least discAtLeast rules for checking discimination number
    // (we track more than discAtLeast if e.g. the first rule predicts more than discAtLeast)
    int trackTo=0;

    Rule* firstRule = nullptr;

   



};


#endif
