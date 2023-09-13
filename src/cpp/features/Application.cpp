#include <map>
#include <omp.h>
#include <memory>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <functional>
#include <chrono>

#include "Application.h"
#include "../core/TripleStorage.h"
#include "../core/RuleStorage.h"
#include "../core/Types.h"
#include "../core/Rule.h"
#include "../core/Globals.h"


void ApplicationHandler::calculateQueryResults(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter, bool dirIsTail){
    // define rule prediction function depending on direction
    typedef bool (Rule::*RulePredFunc)(int, TripleStorage&, QueryResults&, ManySet);
    RulePredFunc predictHeadOrTail;

    if(dirIsTail){
        predictHeadOrTail = &Rule::predictTailQuery;
    }else{
        predictHeadOrTail = &Rule::predictHeadQuery;
    }
    
    // tail query: predict tail given head; vice versa for head query
    RelNodeToNodes& data =  (dirIsTail) ? target.getRelHeadToTails() : target.getRelTailToHeads();
    // relations
    for (const auto& relQueries : data) {
        int relation = relQueries.first;

        if (_cfg_verbose){
            std::cout<<"Applying rules on relation "<<relation<<" for "<<dirIsTail<<" queries (1 is tail)..."<<std::endl;
        }

        // source to correct target entities
        const NodeToNodes& srcToCand = relQueries.second;
        
        // collect keys for parallelization
        // trading of space overhead for runtime (note that the maximal space overhead here is O(num_entities))
        // alternatively you could iterate over all entities in the parallel section
        // index based which would lead to many redundant lookups
        // or you could use std::advance(headToTailquery, i) in an indexed-based for loop
        // but this would reiterate over the map until i in each iteration
        // lastly: simply parallelize for relations instead of queries
        // but note that some relations can make up 30% of the data
        std::vector<int> keys;
        for (const auto& item : srcToCand) {
            keys.push_back(item.first);
        }

        auto& relRules = rules.getRelRules(relation);

        // parallize rule application for each query in target
        #pragma omp parallel //num_threads(4)
        {
            QueryResults qResults(rank_topk);
            ManySet filter;
            #pragma omp for
            for (int i = 0; i < keys.size(); ++i) {
                auto entityToCands = srcToCand.find(keys[i]);
                // this gives us a query, e.g. tail direction: relation(source,?)
                // we apply only once per query
                // entityToCands->second is a set with all the correct answers from target
                int source = entityToCands->first;
                // filtering for train and additionalFilter
                if (rank_filterWtrain){
                    Nodes* trainFilter = nullptr;
                    trainFilter = (!dirIsTail) ? train.getHforTR(source, relation) : train.getTforHR(source, relation);
                    if (trainFilter){
                        filter.addSet(trainFilter);
                    }   
                    
                }
                // always filter with additionalFilter (can be empty)
                Nodes* naddFilter = nullptr;
                naddFilter = (!dirIsTail) ? addFilter.getHforTR(source, relation) : addFilter.getTforHR(source, relation);
                if (naddFilter){
                    filter.addSet(naddFilter);
                }
                // perform rule application
                for (Rule* rule : relRules){
                    (rule->*predictHeadOrTail)(source, train, qResults, filter);
                    int lastSize = 0;
                    int currSize = qResults.size();
                    // if (currSize>=lastSize+rank_topk){
                    //     if (qResults.checkDiscrimination()){
                    //         break;
                    //     }
                    //     lastSize=currSize;
                    // }
                }

                #pragma omp critical
                {   
                    if (saveCandidateRules){
                        // TODO when needed could prevent copy here by using shared pointer
                        if (dirIsTail){
                            tailQcandsRules[relation][source] = qResults.getCandRules();
                        }else{
                            headQcandsRules[relation][source] = qResults.getCandRules();
                        }
                    }
                   
                   if (performAggregation){
                        auto& writeResults = (dirIsTail) ? tailQcandsConfs : headQcandsConfs;
                        if (rank_aggrFunc=="maxplus"){
                            scoreMaxPlus(qResults.getCandRules(), writeResults[relation][source]);
                        }
                        else{
                            throw std::runtime_error("Aggregation function is not recognized in calcualte ranking.");
                        }
                   }
                }
                
                qResults.clear();
                filter.clear();
            }
        }        
    }
}

void ApplicationHandler::aggregateQueryResults(std::string direction){
    auto& queryResults = (direction=="tail") ? tailQcandsRules : headQcandsRules;
    auto& writeResults = (direction=="tail") ? tailQcandsConfs : headQcandsConfs;
    for (auto& queries: queryResults){
            int relation = queries.first;
            std::unordered_map<int, NodeToPredRules>& srcToCand = queries.second;
            for (auto& query: srcToCand){
                int source = query.first; 
                if (rank_aggrFunc=="maxplus"){
                    scoreMaxPlus(query.second, writeResults[relation][source]);
                }else{
                    throw std::runtime_error("Aggregation function is not recognized in calcualte ranking.");
                }
                
            }
    }
}

// note this does not yet filter with target as ranking is performed query based
void ApplicationHandler::makeRanking(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter){
    calculateQueryResults(target, train, rules, addFilter, true);
    calculateQueryResults(target, train, rules, addFilter, false);
}

// query results must have been calculated before and aggregated
void ApplicationHandler::writeRanking(TripleStorage& target, std::string filepath){
    Index* index = target.getIndex();
    RelNodeToNodes& data = target.getRelTailToHeads();
    std::ofstream file(filepath);

    for (auto& relQueries: data){
        int relation = relQueries.first;
        for (auto& srcTocands: relQueries.second){
            int tail = srcTocands.first;
            // true heads
            Nodes& trueHeads = srcTocands.second;
            // we use this direction to iterate over all triples
            // head relation tail is one triple of the target set
            for (int head: trueHeads){
                if (file.is_open()){
                    file<<index->getStringOfNodeId(head)<<" "<<index->getStringOfRelId(relation)<<" "<<index->getStringOfNodeId(tail)<<std::endl;
                    file<<"Heads: ";
                    // write head ranking
                    CandidateConfs& results = headQcandsConfs[relation][tail];
                    for (int i=0; i<results.size(); i++){
                        auto pair = results[i];
                        int predHead = pair.first;
                        double score = pair.second;
                        // filter with target
                        // current predicted head is excluded if its the true answer to some other query
                        if (rank_filterWtarget && !(predHead==head)){
                            if (!(trueHeads.find(predHead)==trueHeads.end())){
                                continue;
                            }
                        }
                        file<<index->getStringOfNodeId(predHead)<<"\t"<<score<<"\t";
                        if (i==rank_topk-1){
                            break;
                        }
                    }
                    // write tail ranking
                    file<<"\nTails: ";
                    //true tails for filtering
                    Nodes& trueTails = target.getRelHeadToTails()[relation][head];
                    results = tailQcandsConfs[relation][head];
                    for (int i=0; i<results.size(); i++){
                        auto pair = results[i];
                        int predTail = pair.first;
                        double score = pair.second;
                        if (rank_filterWtarget && !(predTail==tail)){
                            if (!(trueTails.find(predTail)==trueTails.end())){
                                continue;
                            }
                        }
                        file<<index->getStringOfNodeId(predTail)<<"\t"<<score<<"\t";
                        if (i==rank_topk-1){
                            break;
                        }
                    }
                    file<<"\n";
                }
            }
        }
    }
}

void ApplicationHandler::scoreMaxPlus(
    const NodeToPredRules& candToRules, std::vector<std::pair<int, double>>& aggrCand
     ){
    
    // for noisy-or we can simply sort according to aggrCand after scoring
    // here we have to sort and score separately
    std::vector<std::pair<int, std::vector<Rule*>>> candsToSort(candToRules.begin(), candToRules.end());

    // max+ sorting
    auto sortLexicographic = [](const std::pair<int, std::vector<Rule*>>& candA, const std::pair<int, std::vector<Rule*>>& candB) { 
        std::vector<Rule*> rulesA = candA.second;
        std::vector<Rule*> rulesB = candB.second;

        int minRules = std::min(rulesA.size(), rulesB.size());
        for (int i=0; i<minRules; i++){
            double confA = rulesA[i]->getConfidence(_cfg_nUnseen);
            double confB = rulesB[i]->getConfidence(_cfg_nUnseen);
            if (confA > confB){
                return true;
            } else if (confB > confA){
                return false;
            }
        }
        // all compared rules were equal rank according to num rules
        if (rulesB.size() > rulesA.size()){
            return false;
        } else if (rulesA.size() > rulesB.size()){
            return true;
        }
        // exactly the same rules given NodeToPred is unordered_map return random order
        return false;
    };
     std::sort(candsToSort.begin(), candsToSort.end(), sortLexicographic);
    

    // take sorted candidate and derive its score according to highest rule
     for (auto& pair: candsToSort){
        aggrCand.push_back(
            std::make_pair(
                pair.first,
                pair.second[0]->getConfidence(_cfg_nUnseen)
                )
            );
     }
}

 
void ApplicationHandler::setNumPreselect(int num){
    rank_numPreselect=num;
}

void ApplicationHandler::setTopK(int topk){
    rank_topk=topk;
}
void ApplicationHandler::setFilterWTrain(bool ind){
    rank_filterWtrain=ind;
}

void ApplicationHandler::setFilterWtarget(bool ind){
    rank_filterWtarget = ind;
}
void ApplicationHandler::setAggregationFunc(std::string func){
    rank_aggrFunc = func;
}

void ApplicationHandler::setSaveCandidateRules(bool ind){
    saveCandidateRules = ind;
}
void  ApplicationHandler::setPerformAggregation(bool ind){
    performAggregation = ind;
}

std::unordered_map<int,std::unordered_map<int, NodeToPredRules>>& ApplicationHandler::getHeadQcandsRules(){
    return headQcandsRules;
}
std::unordered_map<int,std::unordered_map<int, CandidateConfs>>&  ApplicationHandler::getHeadQcandsConfs(){
    return headQcandsConfs;
}

std::unordered_map<int,std::unordered_map<int, NodeToPredRules>>&  ApplicationHandler::getTailQcandsRules(){
    return tailQcandsRules;
}
std::unordered_map<int,std::unordered_map<int, CandidateConfs>>& ApplicationHandler::getTailQcandsConfs(){
    return tailQcandsConfs;
}