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





void ApplicationHandler::calculateTripleScores(std::vector<Triple> triples, TripleStorage& train, RuleStorage& rules){

    tripleScores.resize(triples.size());
    if (score_collectGr){
        tripleGroundings.resize(triples.size());
    }
    #pragma omp parallel num_threads(num_thr)
    {
        QueryResults tripleResults(1, 1);
        RuleGroundings ruleGroundings;   
        #pragma omp for
        for (int i=0; i<triples.size(); i++){
           
            if (verbose && i%1000==0 && i>0){
                std::cout<<"Scored 1000 triples..."<<std::endl;
            }
            Triple triple = triples[i];
            int head = triple[0];
            int rel = triple[1];
            int tail = triple[2];
            auto& relRules = rules.getRelRules(rel);
            
                
            //triple = head, rel, tail
            int ctr = 0;
            for (Rule* rule: relRules){
                bool madePred;
                if (score_collectGr){
                    madePred = rule->predictTriple(head, tail, train, tripleResults, &ruleGroundings);
                }else{
                    madePred = rule->predictTriple(head, tail, train, tripleResults, nullptr);
                }
                        
                if (madePred){
                    ctr+= 1;
                }
                if (ctr>=score_numTopRules && score_numTopRules>0){
                    break;
                }
                // potentially this is possible for scoring as maxplus scores are currently the same as max scores
                // however, for groundings tracking we want the num_top_rules parameter to have effect
                // if (rank_aggrFunc=="maxplus" && ctr==1){
                //     break;
                // }

            }
            #pragma omp critical
            {
                        
                // aggregation is performed on tripleResults that only holds one (tail) candidate
                // for triple =head, rel, tail tripleResults holds the tail "candidate" for the triple
                // note that tie handling has no effect, we are just aggregating one candidate

                // the int represents the tail of the triple, vector is not necessary but we keep it simple here
                // and use existing functionality
                std::vector<std::pair<int, double>> score;
                if (rank_aggrFunc=="maxplus"){
                    scoreMaxPlus(tripleResults.getCandRules(), score, train);
                }
                double trScore = 0;
                if (score.size()>0){
                    trScore = score[0].second;
                }
                // for easy conversion later
                tripleScores.at(i) = { (double) triple[0],  (double) triple[1], (double) triple[2], trScore};  
                if (score_collectGr){
                    tripleGroundings.at(i) = std::make_pair(triple, ruleGroundings);
                }    
            }
            tripleResults.clear();
            ruleGroundings.clear();
        }
    }
    tripleResults.clear();
    ruleGroundings.clear();
   }
}

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

        if (verbose){
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
        #pragma omp parallel num_threads(num_thr)
        {
            QueryResults qResults(rank_topk, rank_discAtLeast);
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
                int ctr = 0;
                int currSize = 0;
                for (Rule* rule : relRules){
                    ctr += 1;
                    (rule->*predictHeadOrTail)(source, train, qResults, filter);
                    currSize = qResults.size();
                    if (currSize>=rank_numPreselect){
                        break;
                    }
                    // TODO possibly optimize
                    // checking for discrimination after every rule had no noticeable overhead
                    if (rank_discAtLeast>0 && currSize>=rank_topk){
                        if (qResults.checkDiscrimination()){
                            break;
                        }
                    }
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
                            // tail/headQcandsConfs is filled here
                            scoreMaxPlus(qResults.getCandRules(), writeResults[relation][source], train);
                        }
                        else{
                            throw std::runtime_error("Aggregation function is not recognized in calculate ranking.");
                        }
                   }
                }
                
                qResults.clear();
                filter.clear();
            }
        }        
    }
}

void ApplicationHandler::aggregateQueryResults(std::string direction, TripleStorage& train){
    auto& queryResults = (direction=="tail") ? tailQcandsRules : headQcandsRules;
    auto& writeResults = (direction=="tail") ? tailQcandsConfs : headQcandsConfs;
    for (auto& queries: queryResults){
            int relation = queries.first;
            std::unordered_map<int, NodeToPredRules>& srcToCand = queries.second;
            for (auto& query: srcToCand){
                int source = query.first; 
                if (rank_aggrFunc=="maxplus"){
                    scoreMaxPlus(query.second, writeResults[relation][source], train);
                }else{
                    throw std::runtime_error("Aggregation function is not recognized in calculate ranking.");
                }
                
            }
    }
}

// note this does not yet filter with target as ranking is performed query based
void ApplicationHandler::makeRanking(TripleStorage& target, TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter){
    if (rank_tie_handling=="frequency"){
        if (verbose){
            std::cout<<"Calculate entity frequencies..."<<std::endl;
        }
        
        train.calcEntityFreq();
    }
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
                    CandidateConfs& resultsHead = headQcandsConfs[relation][tail];
                    for (int i=0; i<resultsHead.size(); i++){
                        auto pair = resultsHead[i];
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
                    CandidateConfs& resultsTail = tailQcandsConfs[relation][head];
                    for (int i=0; i<resultsTail.size(); i++){
                        auto pair = resultsTail[i];
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
    const NodeToPredRules& candToRules, std::vector<std::pair<int, double>>& aggrCand, TripleStorage& train
     ){
    
    // for noisy-or we can simply sort according to aggrCand after scoring
    // here we have to sort and score separately
    std::vector<std::pair<int, std::vector<Rule*>>> candsToSort(candToRules.begin(), candToRules.end());

    // max+ sorting
    auto sortLexicographic = [&train, this](const std::pair<int, std::vector<Rule*>>& candA, const std::pair<int, std::vector<Rule*>>& candB) { 
        std::vector<Rule*> rulesA = candA.second;
        std::vector<Rule*> rulesB = candB.second;

        int minRules = std::min(rulesA.size(), rulesB.size());
        for (int i=0; i<minRules; i++){
            double confA = rulesA[i]->getConfidence();
            double confB = rulesB[i]->getConfidence();
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
        //exactly the same rules given NodeToPred is unordered_map return random order
        if (this->rank_tie_handling=="random"){
            return false;
        }else if (this->rank_tie_handling=="frequency"){
            if (train.getFreq(candA.first)!=train.getFreq(candB.first)){
                return train.getFreq(candA.first) > train.getFreq(candB.first);
            }else{
                return candA.first<candB.first;
            }
            
        } else {
            throw std::runtime_error("Could not understand tie_handling_paramter in scoreMaxPlus.");
        }
    };
     std::sort(candsToSort.begin(), candsToSort.end(), sortLexicographic);
    

    // take sorted candidate and derive its score according to highest rule
     for (auto& pair: candsToSort){
        aggrCand.push_back(
            std::make_pair(
                pair.first,
                pair.second[0]->getConfidence()
                )
            );
     }
}

void ApplicationHandler::clearAll(){
    headQcandsRules.clear();
    headQcandsConfs.clear();
    tailQcandsRules.clear();
    tailQcandsConfs.clear();
    tripleScores.clear();
    tripleGroundings.clear();
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

void ApplicationHandler::setDiscAtLeast(int num){
    rank_discAtLeast = num;
}

void ApplicationHandler::setTieHandling(std::string opt){
    rank_tie_handling = opt;
}

void ApplicationHandler::setVerbose(bool ind){
    verbose = ind;
}

void ApplicationHandler::setScoreCollectGroundings(bool ind){
    score_collectGr = ind;
}

void ApplicationHandler::setScoreNumTopRules(int num){
    score_numTopRules = num; 
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

std::vector<std::array<double, 4>>& ApplicationHandler::getTripleScores(){
    return tripleScores;
}
std::vector<std::pair<Triple, RuleGroundings>>& ApplicationHandler::getTripleGroundings(){
    return tripleGroundings;
}

void ApplicationHandler::setNumThr(int num){
    if (num==-1){
        num_thr = omp_get_max_threads();
    }else{
        num_thr = num;
    }
   
}