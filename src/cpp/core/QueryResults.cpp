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
    // new candidate
    if (it==candRules.end()){
        candidateOrder.push_back(cand);
        //discrimination tracking
        if (!firstRule){
            firstRule = rule;
        }
        if (firstRule==rule || trackTo<discAtLeast){
            trackTo+=1;
        }
    }
    // update or insert
    candRules[cand].push_back(rule);
}

void QueryResults::clear(){
    candRules.clear();
    candidateOrder.clear();
    firstRule = nullptr;
    trackTo = 0;
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

bool QueryResults::checkDiscrimination(){
    if (trackTo<discAtLeast){
        return false;
    }
    int numDiscriminated = 0;
    for (int i=1; i<trackTo; i++){
        if (numDiscriminated>=discAtLeast){
            return true;
        }
        int lastCand = candidateOrder[i-1];
        int currentCand = candidateOrder[i];
        std::vector<Rule*>& rulesLast = candRules[lastCand];
        std::vector<Rule*>& rulesCurrent = candRules[currentCand];

        // different amount of rules, can be discriminated
        if (rulesLast.size()!=rulesCurrent.size()){
            numDiscriminated += 1;
            continue;
        // are all rules the same
        } else {
            bool isSame = true;
            for (int j=0; j<rulesCurrent.size(); j++){
                // equality between pointers
                if (rulesLast!=rulesCurrent){
                    numDiscriminated +=1;
                    isSame = false;
                    break;
                }
            }
            if (isSame){
                return false;
            }
        }
    }


}