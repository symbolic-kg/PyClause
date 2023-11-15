#include "RuleStorage.h"
#include "Index.h"
#include "Globals.h"
#include "Rule.h"
#include "Types.h"

#include <fstream>
#include <string>


RuleStorage::RuleStorage(std::shared_ptr<Index> index, std::shared_ptr<RuleFactory> ruleFactory){
    this->ruleFactory = ruleFactory;
    this->index = index;
}

void RuleStorage::addRule(std::unique_ptr<Rule*> rule){
    throw std::runtime_error(" RuleStorage.addRule() Not implemented yet.");
  
}

void RuleStorage::readAnyTimeFormat(std::string path, bool sampled){
    int currID = 0;
	std::string line;
	std::ifstream file(path);
	if (file.is_open()) {
        int IDs = 0;
		while (!util::safeGetline(file, line).eof()){
            if (currID%1000000==0 && verbose && currID>0){
                std::cout<<"...read "<<currID<<" rules "<<std::endl;
            }
            // expects a line: predicted\t cpredicted\trulestring
			std::vector<std::string> splitline = util::split(line, '\t');
            std::string ruleString = splitline[3];
            std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(ruleString);
            if (rule){
                rule->setID(currID);
                rule->setStats(
                    std::stoi(splitline[0]), std::stoi(splitline[1]), false
                );
                rule->setRuleString(ruleString);
                currID+=1;
                relToRules[rule->getTargetRel()].insert(&(*rule)); //same as insert(rule.get())
                rules.push_back(std::move(rule));
            } // else skip
        }
        std::cout<<"Loaded "<<currID<<" rules."<<std::endl;
    }else{
         throw std::ios_base::failure("Could not open rule file: " + path + " is the path correct?");
    }
};



std::set<Rule*, compareRule>&  RuleStorage::getRelRules(int relation){
    return relToRules[relation];
}



std::vector<std::unique_ptr<Rule>>& RuleStorage::getRules(){
    return rules;
 }