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
  

}

// TODO creates undefined behavior and process gets stuck when file does not exist, check first
void RuleStorage::readAnyTimeFormat(std::string path, bool sampled){
    int currID = 0;
	std::string line;
	std::ifstream file(path);
	if (file.is_open()) {
        int IDs = 0;
		while (!util::safeGetline(file, line).eof()){
            // expects a line: predicted\t cpredicted\trulestring
			std::vector<std::string> splitline = util::split(line, '\t');
            std::string ruleString = splitline[3];
            std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(ruleString);
            //std::unique_ptr<Rule> rule = parseAnytimeRule(ruleString);
            if (rule){
                rule->setID(currID);
                rule->setStats(
                    std::stoi(splitline[0]), std::stoi(splitline[1]), _cfg_exactConf
                );
                rule->setRuleString(ruleString);
                currID+=1;
                relToRules[rule->getTargetRel()].insert(&(*rule)); //equivalent to insert(rule.get())
                // rule is nullptr after this
                rules.push_back(std::move(rule));
            }else{
                //std::cout<<"yeah was null\n";
            }
            

            // get statistics here
      
            // makes sense to set the rule string directly from here


            // for (auto el: splitline){
            //     std::cout<<el<<"\n";
            // }
        }
        std::cout<<"Loaded "<<currID<<" rules."<<std::endl;
    }
};



std::set<Rule*, compareRule>&  RuleStorage::getRelRules(int relation){
    return relToRules[relation];
}



std::vector<std::unique_ptr<Rule>>& RuleStorage::getRules(){
    return rules;
 }