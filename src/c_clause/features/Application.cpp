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


    typedef void (ApplicationHandler::*SortAndProcessPtr)(std::vector<std::pair<int,double>>&, QueryResults&, TripleStorage&);
    SortAndProcessPtr sortAndProcess = nullptr;

    if(rank_aggrFunc=="noisyor") {
        sortAndProcess = &ApplicationHandler::sortAndProcessNoisy;
    } else if (rank_aggrFunc=="maxplus") {
        sortAndProcess = &ApplicationHandler::sortAndProcessMax;
    }else{
        throw std::runtime_error("Dont understand the aggregation function.");
    }

    #pragma omp parallel num_threads(num_thr)
    {
        QueryResults tripleResults(1, 1);
        // we dont need to set num_top_rules as the stopping is handled outside; there is only one "candidate"
        tripleResults.setAggrFunc(rank_aggrFunc);
        RuleGroundings ruleGroundings;   
        #pragma omp for schedule(dynamic)
        for (int i=0; i<triples.size(); i++){
           
            if (verbose && i%1000==0 && i>0){
                std::cout<<"Scored "<<(i/1000) * 1000<<" triples..."<<std::endl;
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

            // we actually only have on candidate but we still need to process
            std::unordered_map<int, double>& candScores = tripleResults.getCandScores();
            std::vector<std::pair<int, double>> sortedCandScores(candScores.begin(), candScores.end());
            // tie handling, final processing, sorting
            (this->*sortAndProcess)(sortedCandScores, tripleResults, train);

            #pragma omp critical
            {  
                double trScore = 0;
                if (sortedCandScores.size()>0){
                    trScore = sortedCandScores[0].second;
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

    typedef void (ApplicationHandler::*SortAndProcessPtr)(std::vector<std::pair<int,double>>&, QueryResults&, TripleStorage&);
    SortAndProcessPtr sortAndProcess = nullptr;

    if(rank_aggrFunc=="noisyor") {
        sortAndProcess = &ApplicationHandler::sortAndProcessNoisy;
    } else if (rank_aggrFunc=="maxplus") {
        sortAndProcess = &ApplicationHandler::sortAndProcessMax;
    }else{
        throw std::runtime_error("Dont understand the aggregation function.");
    }



    if (verbose && dirIsTail){
        std::cout<<"Calculating tail queries.."<<std::endl;
    }else if (verbose){
        std::cout<<"Calculating head queries.."<<std::endl;
        
    }

    
    int numNodes = train.getIndex()->getNodeSize();
    int numRel = train.getIndex()->getRelSize();
    // size is num triples not queries 
    int chunk = std::min(10000, std::max(1000, (target.getSize())/50));


    std::vector<std::tuple<int,int,int>> tasks;

    for (int rel=0; rel<numRel; rel++){
        for (int source=0; source<numNodes; source++){
                int* begin;
                int length;
                dirIsTail ? target.getTforHR(source, rel, begin, length) : target.getHforTR(source, rel, begin, length);
                if (length>0){
                    tasks.emplace_back(rel, source, length);
                }
        }
    }
    int ctr=0;
    #pragma omp parallel num_threads(num_thr)
    {
        QueryResults qResults(rank_topk, rank_discAtLeast);
        qResults.setPerformAggregation(performAggregation);
        qResults.setAggrFunc(rank_aggrFunc);
        qResults.setNumTopRules(score_numTopRules);
        ManySet filter;
        #pragma omp for schedule(dynamic)
        for (int i=0; i<tasks.size(); i++){
            int rel = std::get<0>(tasks[i]);
            int source = std::get<1>(tasks[i]);
            int length = std::get<2>(tasks[i]);

            int adapted_topk = rank_topk;   
            if (adapt_topk){
                adapted_topk = rank_topk + length;
                qResults.setAddTopK(adapted_topk);
            }
            ctr+=1;
            if (verbose && ctr%chunk==0 && dirIsTail){
                std::cout<<"Calculated "<< (ctr/chunk) * chunk <<" tail queries..."<<std::endl;
            }else if (verbose && ctr%chunk==0){
                std::cout<<"Calculated "<< (ctr/chunk) * chunk <<" head queries..."<<std::endl;
            }
            auto& relRules = rules.getRelRules(rel);
             // filtering for train and additionalFilter
            if (rank_filterWtrain){
                Nodes* trainFilter = nullptr;
                trainFilter = (!dirIsTail) ? train.getHforTR(source, rel) : train.getTforHR(source, rel);
                if (trainFilter){
                    filter.addSet(trainFilter);
                }   
                        
            }
            // always filter with additionalFilter (can be empty)
            Nodes* naddFilter = nullptr;
            naddFilter = (!dirIsTail) ? addFilter.getHforTR(source, rel) : addFilter.getTforHR(source, rel);
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
                if (rank_numPreselect>0 && currSize>=rank_numPreselect){
                    break;
                }
                // possibly can be optimized
                // checking for discrimination after every rule had no noticeable overhead
                if (currSize>=adapted_topk){
                    if (rank_discAtLeast>0){
                         if (qResults.checkDiscrimination()){
                            break;
                         }
                    }
                    if (score_numTopRules>0){
                        if (qResults.checkNumTopRules()){
                             break;
                        }
                    }
                 }
            }

            std::vector<std::pair<int, double>> sortedCandScores;
            // tie handling, final processing, sorting
            if (performAggregation){
                (this->*sortAndProcess)(sortedCandScores, qResults, train);
            }
                    

            #pragma omp critical
            {   
                if (saveCandidateRules){
                    // TODO when needed could prevent copy here by using shared pointer
                    if (dirIsTail){
                        tailQcandsRules[rel][source] = qResults.getCandRules();
                    }else{
                        headQcandsRules[rel][source] = qResults.getCandRules();
                    }
                }
                if (performAggregation){
                        auto& writeResults = (dirIsTail) ? tailQcandsConfs : headQcandsConfs;
                        writeResults[rel][source] = sortedCandScores;
                }
            }
            qResults.clear();
            filter.clear();
        } 
    } //pragma
}

void ApplicationHandler::sortAndProcessNoisy(std::vector<std::pair<int,double>>& candScoresToSort, QueryResults& qResults, TripleStorage& data){
    // noisyor scoring is already performed in QueryResults

   std::unordered_map<int, double>& candScores = qResults.getCandScores();
   candScoresToSort.assign(candScores.begin(), candScores.end()); 

   if (rank_tie_handling=="random"){
     std::sort(
        candScoresToSort.begin(),
        candScoresToSort.end(), 
        [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            return a.second > b.second;
        }
     );
   }else if (rank_tie_handling=="frequency"){
      std::sort(
        candScoresToSort.begin(),
        candScoresToSort.end(), 
        [&data](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            if (a.second!=b.second){
                return a.second > b.second;
            }else if(data.getFreq(a.first) != data.getFreq(b.first)) {
                return data.getFreq(a.first) > data.getFreq(b.first);
            }else{
                return a.first<b.first;
            }
        }
      );
   }else{
    throw std::runtime_error("Tie handling type not known. Please set to 'random' or 'frequency'");
   }

for (auto& pair: candScoresToSort){
    pair.second = 1 - std::exp(-1*pair.second);
}

}

void ApplicationHandler::sortAndProcessMax(std::vector<std::pair<int,double>>& candScoresToSort, QueryResults& qResults, TripleStorage& data){
    scoreMaxPlus(qResults.getCandRules(), candScoresToSort, data);
}

// currently not used in the ranking process
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

// note this does not yet filter with target as ranking is performed query based; filtering with target only happens when writing the ranking
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
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + filepath );
    }
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
                    int numWritten = 0;
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
                        numWritten += 1;
                        if (numWritten==rank_topk){
                            break;
                        }
                    }
                    // write tail ranking
                    file<<"\nTails: ";
                    //true tails for filtering
                    Nodes& trueTails = target.getRelHeadToTails()[relation][head];
                    CandidateConfs& resultsTail = tailQcandsConfs[relation][head];
                    numWritten = 0;
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
                        numWritten += 1;
                        if (numWritten==rank_topk){
                            break;
                        }
                    }
                    file<<"\n";
                }
            }
        }
    }
    file.close();
    std::cout<<"Ranking file written to:  " + filepath <<std::endl; 
}

// query results must have been calculated before and aggregated
void ApplicationHandler::writeRules(TripleStorage& target, std::string filepath, std::string direction, bool strings){
    
    if ((this->headQcandsRules.size() == 0 && this->tailQcandsRules.size() == 0) || !saveCandidateRules){
        throw std::runtime_error(
            "Please calculate answers using calculate_ranking() and set in the options ranking_handler.collect_rules to true first."
        );
    }

    Index* index = target.getIndex();
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + filepath );
    }

    auto& data = (direction == "head") ? this->headQcandsRules : this->tailQcandsRules;

    for (auto& relQueries: data){
        int relation = relQueries.first;
        std::string relationStr = strings ? "\"" + index->getStringOfRelId(relation) + "\"" : std::to_string(relation);
        for (auto& srcQueries: relQueries.second){
            int src = srcQueries.first;
            std::string srcStr = strings ? "\"" + index->getStringOfNodeId(src) + "\"" : std::to_string(src);
        
            // Collect answers and rules
            std::string answers = "";
            std::string rules = "";

            auto itr = srcQueries.second.begin();
            for(; itr != srcQueries.second.end(); itr++){
                int to = itr->first;
                std::string toStr = strings ? "\"" + index->getStringOfNodeId(to) + "\"" : std::to_string(to);
                answers += toStr;
                if (std::next(itr) != srcQueries.second.end()) {
                    answers += ",";
                }

                std::string ruleset = "[";
                for(int ridx = 0; ridx < itr->second.size(); ridx++){
                    ruleset += strings ? "\"" + itr->second[ridx]->computeRuleString(index) + "\"" : std::to_string(itr->second[ridx]->getID());
                    if (ridx < itr->second.size() - 1){
                        ruleset += ",";
                    }
                }
                ruleset += "]";
                rules += ruleset;
                if (std::next(itr) != srcQueries.second.end()) {
                    rules += ",";
                }
            }
            file << "{\"query\": [" << srcStr << "," << relationStr << "], \"answers\": [" << answers << "], \"rules\": [" << rules << "]}" << std::endl;
        }
    }
    file.close();
    std::cout<<"Rules file written to:  " + filepath <<std::endl; 
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
    if (!(func=="maxplus") && !(func=="noisyor")){
        throw std::runtime_error("The aggregation function value is not known, select from 'noisyor' or 'maxplus' found value: " + func);
    }
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

bool ApplicationHandler::getScoreCollectGroundings(){
    return score_collectGr;
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

void ApplicationHandler::setAdaptTopK(bool ind){
    adapt_topk = ind;
}