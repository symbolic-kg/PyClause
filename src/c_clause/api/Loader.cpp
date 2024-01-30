#include "Loader.h"

#include <functional>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>


 Loader::Loader(std::map<std::string, std::string> options){
    index = std::make_shared<Index>();
    data = std::make_unique<TripleStorage>(index);
    target = std::make_unique<TripleStorage>(index);
    filter = std::make_unique<TripleStorage>(index);
    ruleFactory = std::make_shared<RuleFactory>(index);
    setRuleOptions(options, *ruleFactory);
    rules = std::make_unique<RuleStorage>(index, ruleFactory);
 }

bool Loader::getLoadedData(){
    return loadedData;
}

bool Loader::getLoadedRules(){
    return loadedRules;
}


void Loader::setOptions(std::map<std::string, std::string> options){
    setRuleOptions(options, *ruleFactory);
}


void Loader::loadRules(std::string path){
    rules->clearAll();
    if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFormat(path, false);
    loadedRules = true;
}


void Loader::loadRules(std::vector<std::string> ruleStrings){
    rules->clearAll();
    if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFromVec(ruleStrings, false);
    loadedRules = true;
}


void Loader::loadRules(std::vector<std::string> ruleStrings, std::vector<std::pair<int,int>> ruleStats){
    rules->clearAll();
     if (!loadedData){
         throw std::runtime_error("Please load the data first with the the Handlers load data functionality.");
    }
    rules->readAnyTimeFromVecs(ruleStrings, ruleStats, false);
    loadedRules = true;
}


void Loader::writeRules(std::string path){
    if (!loadedRules){
        throw std::runtime_error("You have to load rules first before you can write them.");
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + path);
    }

    std::vector<std::unique_ptr<Rule>>& loadedRules = this->rules->getRules();
    for (int i=0; i<loadedRules.size(); i++){
        std::array<int, 2> stats = loadedRules[i]->getStats();
        int numPreds = stats[0];
        int numTrue = stats[1];
        double conf = (double) numTrue / (double) numPreds;
        file << numPreds << "\t" << numTrue << "\t" << conf << "\t" << loadedRules[i]->computeRuleString(index.get()) << std::endl;
    }
    std::cout<<"Written rules to:  " + path<<std::endl;
}

std::vector<std::string> Loader::getRuleLines(){
    if (!loadedRules){
        throw std::runtime_error("You have to load rules first before you can write them.");
    }

    std::vector<std::unique_ptr<Rule>>& loadedRules = this->rules->getRules();
    std::vector<std::string> ret;
    for (int i=0; i<loadedRules.size(); i++){
        std::array<int, 2> stats = loadedRules[i]->getStats();
        int numPreds = stats[0];
        int numTrue = stats[1];
        double conf = (double) numTrue / (double) numPreds;
        std::string ruleLine = "";
        ruleLine +=  std::to_string(numPreds) + "\t" + std::to_string(numTrue) + "\t" +
                     std::to_string(conf)     + "\t" + loadedRules[i]->computeRuleString(index.get());
        ret.push_back(ruleLine);
    }
}


std::unordered_map<std::string, int>& Loader::getNodeToIdx(){
    return index->getNodeToIdx();
}


std::unordered_map<std::string, int>& Loader::getRelationToIdx(){
    return index->getRelationToIdx();
}


void Loader::subsEntityStrings(std::map<std::string, std::string>& newNames){
        index->subsEntityStrings(newNames);

    }
void Loader::subsRelationStrings(std::map<std::string, std::string>& newNames){
        index->subsRelationStrings(newNames);
    }

void Loader::setRuleOptions(std::map<std::string, std::string> options, RuleFactory& ruleFactory){
    // rule options:  individual rule options and options of which rules to use
     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        // Z
        {"load_zero_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleZ(util::stringToBool(val));}},
        {"z_weight", [&ruleFactory](std::string val) {ruleFactory.setZconfWeight(std::stod(val));}},
        {"z_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "z");}},
        {"z_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "z");}},
        {"z_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "z");}},
        {"z_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "z");}},
        // C
        {"load_u_c_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleC(util::stringToBool(val));}},
        {"c_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "c");}},
        {"c_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "c");}},
        {"c_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "c");}},
        {"c_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "c");}},
        {"c_max_length", [&ruleFactory](std::string val) {ruleFactory.setCmaxLength(std::stoi(val));}},
        // B
        {"load_b_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleB(util::stringToBool(val));}},
        {"b_max_branching_factor", [&ruleFactory](std::string val) {ruleFactory.setBbranchingFactor(std::stoi(val));}},
        {"b_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "b");}},
        {"b_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "b");}},
        {"b_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "b");}},
        {"b_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "b");}},
        {"b_max_length", [&ruleFactory](std::string val) {ruleFactory.setBmaxLength(std::stoi(val));}},
        // D
        {"load_u_d_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleD(util::stringToBool(val));}},
        {"d_weight", [&ruleFactory](std::string val) {ruleFactory.setDconfWeight(std::stod(val));}},
        {"d_max_branching_factor", [&ruleFactory](std::string val) {ruleFactory.setDbranchingFactor(std::stoi(val));}},
        {"d_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "d");}},
        {"d_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "d");}},
        {"d_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "d");}},
        {"d_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "d");}},
        {"d_max_length", [&ruleFactory](std::string val) {ruleFactory.setDmaxLength(std::stoi(val));}},
        // XXc
        {"load_u_xxc_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXc(util::stringToBool(val));}},
        {"xxc_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "xxc");}},
        {"xxc_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "xxc");}},
        {"xxc_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "xxc");}},
        {"xxc_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "xxc");}},
        // XXd
        {"load_u_xxd_rules", [&ruleFactory](std::string val) {ruleFactory.setCreateRuleXXd(util::stringToBool(val));}},
        {"xxd_num_unseen", [&ruleFactory](std::string val) {ruleFactory.setNumUnseen(std::stoi(val), "xxd");}},
        {"xxd_min_support", [&ruleFactory](std::string val) {ruleFactory.setMinCorrect(std::stoi(val), "xxd");}},
        {"xxd_min_preds", [&ruleFactory](std::string val) {ruleFactory.setMinPred(std::stoi(val), "xxd");}},
        {"xxd_min_conf", [&ruleFactory](std::string val) {ruleFactory.setMinConf(std::stod(val), "xxd");}},
        
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

std::vector<std::string> Loader::getRuleIdx(){
    if (!loadedRules){
        throw std::runtime_error("You cannot obtain a rule index before you loaded rules into the laoder.");
    }
    std::vector<std::unique_ptr<Rule>>& allRules = rules->getRules();
    std::vector<std::string> out(allRules.size());
    for (int i=0; i<allRules.size(); i++){
        Rule* rule = allRules[i].get();
        if (rule->getID() != i){
            throw std::runtime_error("A rule's idx does not match its position. This is an internal; error check the backend.");
        }
        out.at(i) = rule->computeRuleString(index.get());
    }
    return out;
}


TripleStorage& Loader::getData(){
    return *data;
}

TripleStorage& Loader::getFilter(){
    return *filter;
}

TripleStorage& Loader::getTarget(){
    return *target;
}

RuleStorage& Loader::getRules(){
    return *rules;
}

RuleFactory& Loader::getRuleFactory(){
    return *ruleFactory;
}


std::shared_ptr<Index> Loader::getIndex(){
    return index;
}

// loads a file with tab separated string (token) triples
std::unique_ptr<std::vector<Triple>> Loader::loadTriplesToVec(std::string path){

    auto triples = std::make_unique<std::vector<Triple>>();

    // Open the file
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    // Read the file line by line
    std::string line;


    while (!util::safeGetline(file, line).eof()){
		std::istringstream iss(line);
		std::vector<std::string> results = util::split(line, '\t');
		if (results.size() != 3) {
			throw std::runtime_error("Error while reading a file with Triples please check that every line follows tab separeated: head relation tail format. ");
		}
		Triple triple;
        //TODO catch error here when unseen entity appears; for more informative error handling
        triple[0] = index->getIdOfNodestring(results[0]);
        triple[1] = index->getIdOfRelationstring(results[1]);
        triple[2] = index->getIdOfNodestring(results[2]);

        if (!iss.fail() || iss.eof()) {
            triples->push_back(triple);
        }else{
            throw std::runtime_error("Error while reading a file with Triples please check that every line follows tab separeated: head relation tail format. ");
        }
	}
	file.close();
    return std::move(triples);
}


void Loader::setNodeIndex(std::vector<std::string>& idxToNode){
    if (loadedData){
        throw std::runtime_error("You can only set an entity index before you loaded data.");
    }
        index->setNodeIndex(idxToNode);

}
void Loader::setRelIndex(std::vector<std::string>& idxToRel){
    if (loadedData){
        throw std::runtime_error("You can only set a relation index before you loaded data.");
    }
    index->setRelIndex(idxToRel);
}


