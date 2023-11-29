#include "QAHandler.h"


QAHandler::QAHandler(std::map<std::string, std::string> options): BackendHandler(){
    // parent constructor is called before
    setOptions(options);
    ranker.setVerbose(false);
}


void QAHandler::setOptions(std::map<std::string, std::string> options){
    setRankingOptions(options, ranker);
}


//calculate query answers, queries are (sourceEntity, relation)
void QAHandler::calculate_answers(std::vector<std::pair<std::string, std::string>>& queries, std::shared_ptr<DataHandler> dHandler, std::string headOrTail){
    std::vector<std::pair<int, int>> intQueries(queries.size());

    // like this we need later only optimize the idx version of calculate_answers
    index = dHandler->getIndex();
    for (int i=0; i<queries.size(); i++){
        intQueries.at(i) = std::make_pair(index->getIdOfNodestring(queries[i].first), index->getIdOfRelationstring(queries[i].second));
    }
    calculate_answers(intQueries, dHandler, headOrTail);
}


void QAHandler::calculate_answers(std::vector<std::pair<int, int>>& queries, std::shared_ptr<DataHandler> dHandler, std::string headOrTail){
    if (!dHandler->getLoadedData()){
        throw std::runtime_error("You must load data before you can answer questions.");
    }
    if (!dHandler->getLoadedRules()){
        throw std::runtime_error("You must load rules before you can answer questions.");

    }
    answers.clear();
    answers.resize(queries.size());
    index = dHandler->getIndex();
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
        answers.at(i) = candConfs[query.second][query.first];
    }
}


std::vector<std::vector<std::pair<std::string,double>>> QAHandler::getStrAnswers(){
     std::vector<std::vector<std::pair<std::string, double>>> strAnswers(answers.size());
     for (int i=0; i<answers.size(); i++){
        std::vector<std::pair<std::string, double>> queryAnswers;
        for (auto& pair: answers[i]){
            queryAnswers.push_back(std::make_pair(index->getStringOfNodeId(pair.first), pair.second));
        }
        strAnswers.at(i) = queryAnswers;
     }
    return strAnswers;
}


std::vector<std::vector<std::pair<int, double>>> QAHandler::getIdxAnswers(){
    return answers;
}






