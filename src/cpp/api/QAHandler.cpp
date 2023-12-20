#include "QAHandler.h"

#include "functional"


QAHandler::QAHandler(std::map<std::string, std::string> options): BackendHandler(){
    setOptions(options);
    ranker.setVerbose(false);
    setRankingOptions(options, ranker);
}

void QAHandler::setOptionsFrontend(std::map<std::string, std::string> options){
    setOptions(options);
    setRankingOptions(options, ranker);
}


void QAHandler::setOptions(std::map<std::string, std::string> options){
    // register options for ranker

     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    ranker;

    std::vector<OptionHandler> handlers = {
        {"collect_rules", [this](std::string val) {this->setCollectRules(util::stringToBool(val));}},
    };

    for (auto& handler : handlers) {
        auto opt = options.find(handler.name);
        if (opt != options.end()) {
            if (verbose){
                std::cout<< "Setting option "<<handler.name<<" to: "<<opt->second<<std::endl;
            }
            handler.setter(opt->second);
        }
    }

}


//calculate query answers, queries are (sourceEntity, relation)
void QAHandler::calculate_answers(std::vector<std::pair<std::string, std::string>>& queries, std::shared_ptr<Loader> dHandler, std::string headOrTail){
    std::vector<std::pair<int, int>> intQueries(queries.size());

    // like this we need later only optimize the idx version of calculate_answers
    index = dHandler->getIndex();
    for (int i=0; i<queries.size(); i++){
        try {
            intQueries.at(i) = std::make_pair(index->getIdOfNodestring(queries[i].first), index->getIdOfRelationstring(queries[i].second));
        }
        catch(const std::exception& e){
            throw std::runtime_error(
                "An entity or relation in a query is not known, i.e., not loaded with the data"
                "You can only calculate answers for queries where all elements are known: "
                + queries[i].first + " " + queries[i].second
            );
       }

    }
    calculate_answers(intQueries, dHandler, headOrTail);
}


void QAHandler::calculate_answers(std::vector<std::pair<int, int>>& queries, std::shared_ptr<Loader> dHandler, std::string headOrTail){
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
    if (collectRules){
        ranker.setSaveCandidateRules(true);
        // safety measure to bound lifetime of the dHandler which holds the RulesHandler with the rules to this object;
        this->myDHandler = dHandler;
    }
    ranker.makeRanking(target, dHandler->getData(), dHandler->getRules(), dHandler->getFilter());

    // CandidateConfs is  std::vector<pair<int,double>>
    std::unordered_map<int, std::unordered_map<int, CandidateConfs>>& candConfs = isTailQuery ? ranker.getTailQcandsConfs() : ranker.getHeadQcandsConfs();

    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>>* rules;
    if (collectRules){
        rules = isTailQuery ? &ranker.getTailQcandsRules() : &ranker.getHeadQcandsRules();
        queryRules.resize(queries.size());
    }

     for (int i=0; i<queries.size(); i++){
        std::pair<int,int>& query = queries[i];
        // access as candConfs[relation][sourceEntity]
        answers.at(i) = candConfs[query.second][query.first];
        if (collectRules){
            NodeToPredRules candRules = (*rules)[query.second][query.first];
            //for every candidate
            std::vector<std::vector<Rule*>> candOrderRules;
            for (auto& cand_: answers[i]){
                candOrderRules.push_back(candRules[cand_.first]);
            }
            queryRules.at(i) = candOrderRules;
        }
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


std::vector<std::vector<std::vector<int>>> QAHandler::getIdxRules(){
    std::vector<std::vector<std::vector<int>>> out;

    for (int q=0; q<queryRules.size(); q++){
        out.push_back(std::vector<std::vector<int>>());
        for (int c=0; c<queryRules[q].size(); c++){
            out[q].push_back(std::vector<int>());
            for (Rule*& r: queryRules[q][c]){
                out[q][c].push_back(r->getID());
            }
        }
    }
    return out;

}

std::vector<std::vector<std::vector<std::string>>> QAHandler::getStrRules(){
    std::vector<std::vector<std::vector<std::string>>> out;

    for (int q=0; q<queryRules.size(); q++){
        out.push_back(std::vector<std::vector<std::string>>());
        for (int c=0; c<queryRules[q].size(); c++){
            out[q].push_back(std::vector<std::string>());
            for (Rule*& r: queryRules[q][c]){
                out[q][c].push_back(r->computeRuleString(index.get()));
            }
        }
    }
    return out;
}

void QAHandler::setCollectRules(bool ind){
    collectRules = ind;
}









