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
                "An entity or relation in a query is not known, i.e., not loaded with the data."
                " You can only calculate answers for queries where all elements are known: "
                + queries[i].first + " " + queries[i].second
            );
       }

    }
    calculate_answers(intQueries, dHandler, headOrTail);
}


void QAHandler::calculate_answers(std::vector<std::pair<int, int>>& queries, std::shared_ptr<Loader> dHandler, std::string headOrTail){

    this->queries = queries; // cache queries

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



//calculate query answers, queries are (sourceEntity, relation)
void QAHandler::calculate_answers(std::string& queries, std::shared_ptr<Loader> dHandler, std::string headOrTail){
    std::vector<std::pair<std::string, std::string>> stringQueries;
    std::string line;
	std::ifstream file(queries);
	if (file.is_open())
	{
		while (!util::safeGetline(file, line).eof())
		{
            std::vector<std::string> results = util::split(line, '\t');
			if (results.size() == 2) {
                // rules with stats
                stringQueries.push_back(std::make_pair(results[0], results[1]));
            } else {
				std::cout << 
                    "Unsupported Filetype, please make sure that every line either \
                    contains a properly formatted query: node\trelation" 
                << std::endl;
                exit(-1);
            }
		}
		file.close();
	}
	else {
		std::cout << "Unable to open file " << queries << std::endl;
		exit(-1);
	}
    calculate_answers(stringQueries, dHandler, headOrTail);
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


void QAHandler::writeAnswers(std::string outputPath, bool strings){

    if (this->queries.size() == 0){
        throw std::runtime_error(
            "Please calculate answers using calculate_answers() first."
        );
    }
    
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + outputPath);
    }

    for (int idx = 0; idx < this->queries.size(); idx++){
        
        std::string source = strings ? "\"" + index->getStringOfNodeId(this->queries[idx].first) + "\"" : std::to_string(this->queries[idx].first);
        std::string rel = strings ? "\"" + index->getStringOfRelId(this->queries[idx].second) + "\"" : std::to_string(this->queries[idx].second);
        
        // Collect answers and scores
        std::string answers = "";
        std::string scores = "";
        auto itr = this->answers[idx].begin();
        for (; itr != this->answers[idx].end(); itr++){
            answers += strings ? "\"" + index->getStringOfNodeId(itr->first) + "\"" : std::to_string(itr->first);
            scores += std::to_string(itr->second);
            if (std::next(itr) != this->answers[idx].end()) {
                answers += ",";
                scores += ",";
            }
        }
        
        // Write row
        file << "{\"query\": [" << source << "," << rel << "], \"answers\": [" << answers << "], \"scores\": [" << scores << "]}" << std::endl;
    }
    file.close();
}


std::vector<std::vector<std::vector<int>>> QAHandler::getIdxRules(){
    if (!collectRules){
        throw std::runtime_error("Please set 'qa_handler.collect_rules' to true before you calculate answers");
    }
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
    if (!collectRules){
        throw std::runtime_error("Please set 'qa_handler.collect_rules' to true before you calculate answers");
    }
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


void QAHandler::writeRules(std::string outputPath, bool strings){

    if (this->queries.size() == 0 || !collectRules){
        throw std::runtime_error(
            "Please calculate answers using calculate_answers() and set in the options qa_handler.collect_rules to true first."
        );
    }
    
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + outputPath);
    }

    for (int idx = 0; idx < this->queries.size(); idx++){
        
        std::string source = strings ? "\"" + index->getStringOfNodeId(this->queries[idx].first) + "\"" : std::to_string(this->queries[idx].first);
        std::string rel = strings ? "\"" + index->getStringOfRelId(this->queries[idx].second) + "\"" : std::to_string(this->queries[idx].second);
        
        // Collect answers and scores
        std::string answers = "";
        auto itr = this->answers[idx].begin();
        for (; itr != this->answers[idx].end(); itr++){
            answers += strings ? "\"" + index->getStringOfNodeId(itr->first) + "\"" : std::to_string(itr->first);
            if (std::next(itr) != this->answers[idx].end()) {
                answers += ",";
            }
        }

        std::string rules = "";
        for (int rs_ix = 0; rs_ix < queryRules[idx].size(); rs_ix++){
            std::string ruleset = "[";
            for (int r_ix = 0; r_ix < queryRules[idx][rs_ix].size(); r_ix++){
               ruleset += strings ? "\"" + queryRules[idx][rs_ix][r_ix]->computeRuleString(index.get()) + "\"" : std::to_string(queryRules[idx][rs_ix][r_ix]->getID());
               if (r_ix < queryRules[idx][rs_ix].size()-1){
                    ruleset += ",";
               }
            }
            ruleset += "]";

            // add ruleset list to rules list
            rules += ruleset;
            if (rs_ix < queryRules[idx].size()-1){
                rules += ",";
            }
        }
        
        // Write row
        file << "{\"query\": [" << source << "," << rel << "], \"answers\": [" << answers << "], \"rules\": [" << rules << "]}" << std::endl;
    }
    file.close();
}

void QAHandler::setCollectRules(bool ind){
    collectRules = ind;
}









