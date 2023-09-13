#include "QueryResults.h"
#include <iostream>


QueryResults::QueryResults(int addTopK){
    this->addTopK = addTopK;
}


void QueryResults::insertRule(int cand, Rule* rule){
    if (addTopK > 0 && candidateOrder.size()>= addTopK){
        return;
    }

    auto it = candRules.find(cand);
    if (it==candRules.end()){
        candidateOrder.push_back(cand);
    }
    candRules[cand].push_back(rule);
}

void QueryResults::clear(){
    candRules.clear();
    candidateOrder.clear();
}

std::vector<Rule*>& QueryResults::getRulesForCand(int cand){
    return candRules[cand];
}

NodeToPredRules& QueryResults::getCandRules(){
    return candRules;
}

int QueryResults::size(){
    return candidateOrder.size();
}

bool QueryResults::contains(int cand){
    if (candRules.find(cand)!=candRules.end()){
        return true;
    }else{
        return false;
    }
}

bool QueryResults::empty(){
    return candidateOrder.empty();
}