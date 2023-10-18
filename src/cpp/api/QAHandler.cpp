#include "QAHandler.h"


QAHandler::QAHandler(std::map<std::string, std::string> options): BackendHandler(){
    filter = std::make_unique<TripleStorage>(index);
    // parent constructor is called before
    setRuleOptions(options, *ruleFactory);
    setRankingOptions(options, ranker);
    ranker.setVerbose(false);
}

void QAHandler::loadDatasets(std::string dataPath, std::string filterPath){
     if (loadedData){
        throw std::runtime_error("Please load the data only once or use a new handler.");
    }

    std::cout<<"Load data... \n";
    if (filterPath!=""){
        filter->read(filterPath);
    }
    
    // sets loadedData to true
    loadData(dataPath);
}

//queries are (sourceEntity, relation)
 std::vector<std::vector<std::pair<int, double>>> QAHandler::answerQueries(std::vector<std::pair<int, int>> queries, std::string headOrTail){
    if (!loadedData){
        throw std::runtime_error("You must load data before you can answer questions.");
    }
    if (!loadedRules){
        throw std::runtime_error("You must load rules before you can answer questions.");

    }
    std::vector<std::vector<std::pair<int, double>>> ret(queries.size());
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
    ranker.makeRanking(target, *data, *rules, *filter);

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
 std::vector<std::vector<std::pair<std::string, double>>> QAHandler::answerQueries(std::vector<std::pair<std::string, std::string>> queries, std::string headOrTail){
    if (!loadedData){
        throw std::runtime_error("You must load data before you can answer questions.");
    }
    if (!loadedRules){
        throw std::runtime_error("You must load rules before you can answer questions.");

    }
    std::vector<std::vector<std::pair<std::string, double>>> ret(queries.size());
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
        isTailQuery ?  target.add(query.first, query.second, index->getStringOfNodeId(d)) : target.add(index->getStringOfNodeId(d), query.second, query.first);
    }
    ranker.makeRanking(target, *data, *rules, *filter);

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



