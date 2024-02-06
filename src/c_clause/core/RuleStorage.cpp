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


// reads format outputted by AnyBURL
void RuleStorage::readAnyTimeFormat(std::string path, bool exact){
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::ios_base::failure("Could not open rule file: " + path + " is the path correct?");
    }

    if (verbose){
        std::cout << "Loading rules from " + path << std::endl;
    }

    std::ios_base::sync_with_stdio(false); 
    
    constexpr size_t bufferSize =  256 * 1024; 
    char buffer[bufferSize];
    file.rdbuf()->pubsetbuf(buffer, bufferSize);

    std::string line;
    int currID = 0;
    int currLine = 0;
    
    while (!util::safeGetline(file, line).eof()){
        if (currLine % 1000000 == 0 && verbose && currLine > 0){
            std::cout << "...parsed " << currLine << " rules " << std::endl;
        }
        bool added = addAnyTimeRuleLine(line, currID, false);
        if (added){
            currID += 1;
        }
        currLine += 1;
    }
    std::cout << "Loaded " << currID << " rules." << std::endl;
}

// ruleStrings is a line num_pred/t support/t conf/t ruleString
void RuleStorage::readAnyTimeFromVec(std::vector<std::string>& ruleStrings, bool exact){
    int currID = 0;
    for (int i=0; i<ruleStrings.size(); i++){
         if (i%1000000==0 && verbose && i>0){
                std::cout<<"...serialized "<<i<<" rules "<<std::endl;
        }
        std::string stringLine = ruleStrings[i];
        bool added = addAnyTimeRuleLine(stringLine, currID, exact);
        if (added){
            currID += 1;
        }
    }
    std::cout<<"Loaded "<<currID<<" rules."<<std::endl;
} 

void RuleStorage::readAnyTimeFromVecs(std::vector<std::string>& ruleStrings, std::vector<std::pair<int,int>> stats, bool exact){
    if (ruleStrings.size() != stats.size()){
        throw std::runtime_error(
            "The rule stats input list must have same length of rule string list when loading rules with stats."
        );
    }
    int currID = 0;
    for (int i=0; i<ruleStrings.size(); i++){
        if (i%1000000==0 && verbose && i>0){
                std::cout<<"...serialized "<<i<<" rules "<<std::endl;
        }
        std::string stringRule = ruleStrings[i];
        int numPred = stats[i].first;
        int numTrue = stats[i].second;
        bool added = addAnyTimeRuleWithStats(stringRule, currID, numPred, numTrue, exact);
        if (added){
            currID += 1;
        }
    }
    std::cout<<"Loaded "<<currID<<" rules."<<std::endl;

}

bool RuleStorage::addAnyTimeRuleLine(std::string ruleLine, int id , bool exact){
    // expects a line: predicted\t cpredicted\tconf\trulestring
	std::vector<std::string> splitline = util::split(ruleLine, '\t');

    if (splitline.size()==1){
        int numPreds = 1;
        int numTrue = 1;
        std::cout<<"Warning: could not find num preds and support for input line " + splitline[0]<<std::endl;
        std::cout<<" Setting both to 1. Expect random ordering for rules and predictions, confidence scores will all be 1."<<std::endl;
        return addAnyTimeRuleWithStats(splitline[0], id, numPreds, numTrue, exact);
    }

    if (splitline.size()!=4){
        throw std::runtime_error("Could not parse this rule because of format: " + ruleLine);
    }
    std::string ruleString = splitline[3];
    int numPreds = std::stoi(splitline[0]);
    int numTrue = std::stoi(splitline[1]);

    return addAnyTimeRuleWithStats(ruleString, id, numPreds, numTrue, exact);
}

bool RuleStorage::addAnyTimeRuleWithStats(std::string ruleString, int id, int numPred, int numTrue, bool exact){
    std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(ruleString, numPred, numTrue);
    if (rule){
        rule->setID(id);
        rule->setStats(numPred, numTrue, exact);
        relToRules[rule->getTargetRel()].insert(rule.get());
        rules.push_back(std::move(rule));
        return true;
    } else {
        return false;
    }
}


std::set<Rule*, compareRule>&  RuleStorage::getRelRules(int relation){
    return relToRules[relation];
}



std::vector<std::unique_ptr<Rule>>& RuleStorage::getRules(){
    return rules;
 }

void RuleStorage::clearAll(){
    rules.clear();
    relToRules.clear();
}