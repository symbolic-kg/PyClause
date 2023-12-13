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
    QueryResults(int addTopK, int discAtLeast);
    QueryResults(){};
    //updates if key exists, adds if it doesnt
    void insertRule(int cand, Rule* rule);
    int size();
    bool contains(int cand);
    bool empty();
    //delete all data
    void clear();
    std::vector<Rule*>& getRulesForCand(int cand);
    NodeToPredRules& getCandRules();
    std::unordered_map<int, double>& getCandScores();
    std::vector<int>& getCandsOrdered();
    // checks if at least discAtLeast top candidates can be fully disciminated
    bool checkDiscrimination();

    bool checkNumTopRules();

    void setAggrFunc(std::string name);
    void setNumTopRules(int num);

    


private:
    // stores vector of rules (pointers) for every candidate
    //typedef NodeToPredRules:  std::unordered_map<int, std::vector<Rule*>> NodeToPredRules;
    NodeToPredRules candRules;
    // tracks insertion order of candidates elements are candidate idx's
    std::vector<int> candidateOrder;

    // aggregation scores
    std::unordered_map<int, double> candScores;

    //**options**
    // maximal number of candidates to add modifies data storing
    int addTopK=-1;
    // the num of top candidates which at least have to be fully disciminated 
    // does not change data storing only affects this->discriminate()
    int discAtLeast=10;

    bool performAggregation = true;
    std::string aggregationFunction = "maxplus";
    // stop updating a candiate if it was predicted already by num_top_rules
    // -1 for off
    int num_top_rules = -1;

    //**internal**
    // we track at least discAtLeast rules for checking discimination number
    // (we track more than discAtLeast if e.g. the first rule predicts more than discAtLeast)
    int trackTo=0;
    int numDiscriminated=0;

    Rule* firstRule = nullptr;
    Rule* currentRule = nullptr;

   



};


#endif
