#include <map>
#include <omp.h>
#include <memory>

#include "Ranking.h"
#include "../core/TripleStorage.h"
#include "../core/RuleStorage.h"
#include "../core/Types.h"
#include "../core/Rule.h"
#include "../core/Globals.h"


void RankingHandler::calculateQueryResults(TripleStorage& train, TripleStorage& target, RuleStorage& rules, TripleStorage& addFilter){
    std::cout<<"hi";


    #pragma omp parallel for num_threads(2)
    for(int i=0; i< 3; i++) {
        for (int j=0; j< 3; j++) {
        printf("i = %d, j= %d, threadId = %d \n", i, j, omp_get_thread_num());
    }
}


    // tail query: predict tail given head
    RelNodeToNodes& tailQueries = target.getRelHeadToTails();
    std::cout<<"start tail queries"<<std::endl;
    // relations
    
    for (const auto& relTQueries: tailQueries){
        int relation = relTQueries.first;
        const NodeToNodes& headToTail = relTQueries.second;
        // queries per relation
        for (auto& headPairs: headToTail){
            // this gives us a query: relation(head,?)
            // we apply only once per query
            // headPairs.second is a set with all the tails from train
            int head = headPairs.first;
            NodeToPredRules queryResults;
            ManySet filter;
            filter.addSet(&headPairs.second);
            Nodes* naddFilter = nullptr;
            naddFilter = addFilter.getTforHR(head, relation);
            if (naddFilter){
                filter.addSet(naddFilter);
            }
            for (Rule* rule: rules.getRelRules(relation)){
                rule->predictTailQuery(head, train, queryResults, filter);
                if (queryResults.size()>_cfg_rnk_numPreselect){
                   break;
                }
            }
            tailQueryResults[relation][head] = queryResults;
            
        }
        
    }
    std::cout<<"end head"<<std::endl;

    std::cout<<"start head queries"<<std::endl;
    RelNodeToNodes& headQueries = target.getRelTailToHeads();
    // all relations
    for (const auto& relHQueries: headQueries){
        int relation = relHQueries.first;
        const NodeToNodes& tailToHead = relHQueries.second;
        // all queries
        for (const auto& tailPairs:tailToHead){
            // this gives us a query: relation(?,tail)
            // we apply only once per query
             //headPairs.second is a set with all the true heads
            int tail = tailPairs.first;
            ManySet filter;
            filter.addSet(&tailPairs.second);
            Nodes* naddFilter = nullptr;
            naddFilter = addFilter.getHforTR(tail, relation);
            if (naddFilter){
                filter.addSet(naddFilter);
            }
            NodeToPredRules queryResults;
            for (Rule* rule: rules.getRelRules(relation)){
                rule->predictHeadQuery(tail, train, queryResults, filter);
                if (queryResults.size()>_cfg_rnk_numPreselect){
                    break;
                }
            }
            tailQueryResults[relation][tail] = queryResults;

            
            

        }
    }
    std::cout<<"end tail"<<std::endl;



}

void RankingHandler::scoreMaxPlus(NodeToPredRules queryResults, std::map<int,double>){

    


}