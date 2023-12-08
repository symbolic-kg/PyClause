#include "QueryResults.h"
#include <iostream>
#include <cmath>
#include "Rule.h"

QueryResults::QueryResults(int addTopK, int discAtLeast){
    this->addTopK = addTopK;
    // discriminating 2 means only one check
    this->discAtLeast = discAtLeast-1;
    if (this->discAtLeast==0){
        std::runtime_error("Check option 'disc_at_least'");
    }
}


void QueryResults::insertRule(int cand, Rule* rule){
    // topk is reached dont add new candidates but always add everything from the first rule
    // to not cut off results randomly at the beginning
    // also add all candidates from the current rule to not randomly cut off rule predictions
    bool onlyUpdate = (addTopK > 0 && candidateOrder.size()>= addTopK && rule != firstRule && currentRule!=rule);
    // as long as we not have topk we track the current rule to not cut candidats off
    if (addTopK>candidateOrder.size()){
         currentRule = rule;
    }
    auto it = candRules.find(cand);
    bool newCand = (it==candRules.end());
    // new candidate
    if (newCand && !onlyUpdate ){
        candidateOrder.push_back(cand);
        // discrimination tracking
        if (!firstRule && discAtLeast>0){
            firstRule = rule;
        }
        // discriminates always all candidates of the first rule
        // assumes insertion order regarding rules
        if (firstRule==rule || ( discAtLeast>0 && trackTo<discAtLeast)){
            trackTo+=1;
        }
    }
    // update or insert
    // known cand: always update
    // new cand: -> only add and update when explicitly asked by !onlyUpdate
    if (!onlyUpdate || !newCand){
        candRules[cand].push_back(rule);

        // we added a new rule, update aggregation score
        if (performAggregation){
            if (aggregationFunction=="noisy-or"){
                candScores[cand] += std::log(1-rule->getConfidence());
            } else if (aggregationFunction=="maxplus"){
                int n = candRules[cand].size();
                // TODO simply threshold this
                double mult = std::pow(0.001, n-1);
                candScores[cand] += mult * rule->getConfidence();
            }
        }
    }
}

std::unordered_map<int,double>& QueryResults::getCandScores(){
    return candScores;
}

void QueryResults::clear(){
    candRules.clear();
    candidateOrder.clear();
    firstRule = nullptr;
    currentRule = nullptr;
    trackTo = 0;
    numDiscriminated = 0;
    candScores.clear();
}

std::vector<int>& QueryResults::getCandsOrdered(){
    return candidateOrder;
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
    // check the non-discriminated pairs if they are discrimonated meanwhile
    // note: two rules that are discriminated once stay discriminated (assuming rule ordering)
    for (int i=numDiscriminated+1; i<trackTo; i++){
        if (numDiscriminated>=discAtLeast){
            return true;
        }
        int lastCand = candidateOrder[i-1];
        int currentCand = candidateOrder[i];
        std::vector<Rule*>& rulesLast = candRules[lastCand];
        std::vector<Rule*>& rulesCurrent = candRules[currentCand];
        
        if //rulesets are equal
        (rulesLast.size() == rulesCurrent.size() && std::equal(rulesCurrent.begin(), rulesCurrent.end(), rulesLast.begin(), rulesLast.end()))
        {
            return false;
        }else{
            numDiscriminated += 1;
        }
    }
}