#include <map>
#include <omp.h>
#include <memory>
#include <algorithm>

#include "Application.h"
#include "../core/TripleStorage.h"
#include "../core/RuleStorage.h"
#include "../core/Types.h"
#include "../core/Rule.h"
#include "../core/Globals.h"






// todo split in head and tail results s.t. that user can calculate one direction
void ApplicationHandler::calculateTQueryResults(TripleStorage& train, RuleStorage& rules, TripleStorage& addFilter){
   // tail query: predict tail given head
    RelNodeToNodes& tailQueries = target.getRelHeadToTails();
    std::cout<<"start tail queries"<<std::endl;
    // relations
    for (const auto& relTQueries : tailQueries) {
        int relation = relTQueries.first;
        const NodeToNodes& headToTail = relTQueries.second;
        
        // collect keys for parallelization
        // trading of space overhead for runtime (note that the maximal space overhead here is O(num_entities))
        // alternatively you could iterate over all entities in the parallel section
        // index based which would lead to many redundant lookups
        // or you could use std::advance(headToTailquery, i) in an indexed-based for loop
        // but this would reiterate over the map until i in each iteration
        // lastly: simply parallelize for relations instead of queries
        // but note that some relations can make up 30% of the data
        std::vector<int> keys;
        for (const auto& item : headToTail) {
            keys.push_back(item.first);
        }

        // parallize rule application for each query in target
        #pragma omp parallel
        {
            NodeToPredRules candRules;
            ManySet filter;
            #pragma omp for nowait
            for (int i = 0; i < keys.size(); ++i) {
                auto headPairs = headToTail.find(keys[i]);
                // this gives us a query: relation(head,?)
                // we apply only once per query
                // headPairs->second is a set with all the correct tails from train
                int head = headPairs->first;
                // filtering for train and additionalFilter
                if (_cfg_rnk_filterWtrain){
                    filter.addSet(&headPairs->second);
                }
                // always filtered with additionalFilter
                Nodes* naddFilter = nullptr;
                naddFilter = addFilter.getTforHR(head, relation);
                if (naddFilter){
                    filter.addSet(naddFilter);
                }
                for (Rule* rule : rules.getRelRules(relation)){
                    rule->predictTailQuery(head, train, candRules, filter);
                    if (candRules.size() > _cfg_rnk_numPreselect){
                    break;
                    }
                }

                #pragma omp critical
                {
                    tailQueryResults[relation][head].candToRules = candRules;
                }
                candRules.clear();
                filter.clear();
            }
        }        
        }
    std::cout<<"end tail"<<std::endl;

    std::cout<<"start head queries"<<std::endl;
    RelNodeToNodes& headQueries = target.getRelTailToHeads();
    // all relations
    for (const auto& relHQueries: headQueries){
        int relation = relHQueries.first;
        const NodeToNodes& tailToHead = relHQueries.second;
        
        // collect keys for parallelization
        std::vector<int> keys;
        for (const auto& item : tailToHead) {
            keys.push_back(item.first);
        }

        #pragma omp parallel
        {
            NodeToPredRules candRules;
            ManySet filter;
            #pragma omp for nowait
            for (int i = 0; i < keys.size(); ++i) {
                auto tailPairs = tailToHead.find(keys[i]);
                int tail = tailPairs->first;
                if (_cfg_rnk_filterWtrain){
                    filter.addSet(&tailPairs->second);
                }
                
                Nodes* naddFilter = nullptr;
                naddFilter = addFilter.getHforTR(tail, relation);
                if (naddFilter){
                    filter.addSet(naddFilter);
                }
                for (Rule* rule: rules.getRelRules(relation)){
                    rule->predictHeadQuery(tail, train, candRules, filter);
                    if (candRules.size() > _cfg_rnk_numPreselect){
                    break;
                    }
                }
                
                #pragma omp critical
                {
                    headQueryResults[relation][tail].candToRules = candRules;
                }
                candRules.clear();
                filter.clear();
            }
        }
    }

    std::cout<<"end head"<<std::endl;

    aggregateQueryResults();
    std::cout<<"aggregated.";
}

void ApplicationHandler::aggregateQueryResults(){
    //tail queries
    for (auto& relQuery: tailQueryResults){
            int relation = relQuery.first;
            std::unordered_map<int, QueryResults>& headToTail = relQuery.second;
            for (auto& query: headToTail){
                int source = query.first;
                QueryResults& results = query.second;
                scoreMaxPlus(results.candToRules, results.aggrCand);
            }
    }
    //head queries
    for (auto& relQuery: headQueryResults){
            int relation = relQuery.first;
            std::unordered_map<int, QueryResults>& tailToHead = relQuery.second;
            for (auto& query: tailToHead){
                int source = query.first;
                QueryResults& results = query.second;
                scoreMaxPlus(results.candToRules, results.aggrCand);
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

