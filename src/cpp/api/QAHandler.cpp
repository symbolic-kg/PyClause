#include "QAHandler.h"


QAHandler::QAHandler(std::map<std::string, std::string> options): BackendHandler(){
    // parent constructor is called before
    setOptions(options);
    ranker.setVerbose(false);
}


void QAHandler::setOptions(std::map<std::string, std::string> options){
    setRankingOptions(options, ranker);
}


//queries are (sourceEntity, relation)
 std::vector<std::vector<std::pair<int, double>>> QAHandler::answerQueries(std::vector<std::pair<int, int>> queries, std::shared_ptr<DataHandler> dHandler, std::string headOrTail){
    if (!dHandler->getLoadedData()){
        throw std::runtime_error("You must load data before you can answer questions.");
    }
    if (!dHandler->getLoadedRules()){
        throw std::runtime_error("You must load rules before you can answer questions.");

    }
    std::vector<std::vector<std::pair<int, double>>> ret(queries.size());
    std::shared_ptr<Index> index = dHandler->getIndex();
    ranker.clearAll();
    TripleStorage target(index);
    bool isTailQuery;
    if (headOrTail=="tail"){
        isTailQuery = true;
    }else if (headOrTail=="head"){
        isTailQuery = false;
    }else{
        throw std::runtime_error("Please specify 'head' or 'tail' as second argument.");
    }
    //query.first is source entity
    //query.second is relation
    for (int i=0; i<queries.size(); i++){
        std::pair<int,int>& query = queries[i];
        isTailQuery ?  target.addIdx(query.first, query.second, 0) : target.addIdx(0, query.second, query.first);
    }
    // note that loadCSR does not break anything for the other CSRs as we only allow already existing idx's (entities and relations)
    target.loadCSR();
    ranker.makeRanking(target, dHandler->getData(), dHandler->getRules(), dHandler->getFilter());

    // CandidateConfs is  std::vector<pair<int,double>>
    std::unordered_map<int, std::unordered_map<int, CandidateConfs>>& candConfs = isTailQuery ? ranker.getTailQcandsConfs() : ranker.getHeadQcandsConfs();

     for (int i=0; i<queries.size(); i++){
        std::pair<int,int>& query = queries[i];
        // access as candConfs[relation][sourceEntity]
        ret.at(i) = candConfs[query.second][query.first];
    }

    

    return ret;
 }


//queries are (sourceEntity, relation)
 std::vector<std::vector<std::pair<std::string, double>>> QAHandler::answerQueries(std::vector<std::pair<std::string, std::string>> queries, std::shared_ptr<DataHandler> dHandler , std::string headOrTail){
     if (!dHandler->getLoadedData()){
        throw std::runtime_error("You must load data before you can answer questions.");
    }
    if (!dHandler->getLoadedRules()){
        throw std::runtime_error("You must load rules before you can answer questions.");

    }
    ranker.clearAll();
    std::vector<std::vector<std::pair<std::string, double>>> ret(queries.size());
    std::shared_ptr<Index> index = dHandler->getIndex();
    TripleStorage target(index);
    bool isTailQuery;
    if (headOrTail=="tail"){
        isTailQuery = true;
    }else if (headOrTail=="head"){
        isTailQuery = false;
    }else{
        throw std::runtime_error("Please specify 'head' or 'tail' as second argument.");
    }
    //query.first is source entity
    //query.second is relation
    int d = 0;
    for (int i=0; i<queries.size(); i++){
        std::pair<std::string,std::string>& query = queries[i];
        // error handling
        try {
             index->getIdOfNodestring(query.first);
        } catch(const std::exception& e){
                throw std::runtime_error("Could not find entity in index: " + query.first + ". If you want to make predictions for unseen entities add them to the index before loading the data.");
        }
        try {
            index->getIdOfRelationstring(query.second);
        } catch (const std::exception& e){
            throw std::runtime_error("Relation " + query.second + " does not exist.");
        }
       
        isTailQuery ?  target.add(query.first, query.second, index->getStringOfNodeId(d)) : target.add(index->getStringOfNodeId(d), query.second, query.first);
    }
    target.loadCSR();
    ranker.makeRanking(target, dHandler->getData(), dHandler->getRules(), dHandler->getFilter());

    // CandidateConfs is  std::vector<pair<int,double>>
    std::unordered_map<int, std::unordered_map<int, CandidateConfs>>& candConfs = isTailQuery ? ranker.getTailQcandsConfs() : ranker.getHeadQcandsConfs();

     for (int i=0; i<queries.size(); i++){
        std::pair<std::string,std::string>& query = queries[i];
        int source = index->getIdOfNodestring(query.first);
        int rel = index->getIdOfRelationstring(query.second);
        // access as candConfs[relation][sourceEntity]
        std::vector<std::pair<std::string, double>> stringCandConfs;
        for (int j=0; j<candConfs[rel][source].size(); j++){
            std::pair<int,double> candConf = candConfs[rel][source].at(j);
            stringCandConfs.push_back({index->getStringOfNodeId(candConf.first), candConf.second});
        }
        ret.at(i) = stringCandConfs;
    }

   

    return ret;
 }



