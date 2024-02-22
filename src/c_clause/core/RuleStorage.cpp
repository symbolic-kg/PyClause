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

void RuleStorage::readAnyTimeParFormat(std::string path, bool exact, int numThreads){
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
    int currLine = 0;

    std::vector<std::string> ruleLines;
    std::vector<std::unique_ptr<Rule>> rules_ptr;


    while (!util::safeGetline(file, line).eof()){
        ruleLines.push_back(line);
    }
    file.close();

    // we not need all of them 
    rules_ptr.resize(ruleLines.size());

    #pragma omp parallel num_threads(numThreads)
    {
        #pragma omp for //no need for dynamic
        for (int i=0; i<ruleLines.size(); i++){

            if (i>0 && i%1000000==0){
                std::cout<<"parsed 1 million rules..." <<std::endl;
            }

            std::string ruleLine = ruleLines[i];

            // expects a line: predicted\t cpredicted\tconf\trulestring
            std::vector<std::string> splitline = util::split(ruleLine, '\t');

            if (splitline.size()==1){
                int numPreds = 100;
                int numTrue = 100;
                std::cout<<"Warning: could not find num preds and support for input line " + splitline[0]<<std::endl;
                std::cout<<" Setting both to 100. Expect random ordering for rules and predictions, confidence scores will all be 1."<<std::endl;
            }

            if (splitline.size()!=4){
                std::cout<<"Could not parse this rule because of line format: " + ruleLine<<std::endl;
                std::cout<<"Skipping but please check your format."<<std::endl;

            }
            std::string ruleString = splitline[3];
            int numPreds = std::stoi(splitline[0]);
            int numTrue = std::stoi(splitline[1]);
            std::unique_ptr<Rule> rule = nullptr;
            try
                {
                    rule = ruleFactory->parseAnytimeRule(ruleString, numPreds, numTrue);
                }
            catch(const std::exception& e)
                {
                    std::cout<<"Could not parse this rule " + ruleLine<<std::endl;
                    std::cout<<"Skipping it, but please check your format."<<std::endl;
                    std::cout<<"And check that if entities are contained, that they are loaded with the data."<<std::endl;
                }

            if (rule){
                rule->setStats(numPreds, numTrue, exact);
                rules_ptr.at(i) = std::move(rule);              
            }
        }
    }
    // need this for correctly setting ID's
    // e.g. we want ID's to be the line order (minus skipped)
    // to be consistent when rules are written and loaded again
    std::cout<< "Indexing rules.." <<std::endl;
    int currID = 0;
    for (int i=0; i<rules_ptr.size(); i++){
        if (rules_ptr[i]){
            rules_ptr[i]->setID(currID);
            // must be done after id is set
            relToRules[rules_ptr[i]->getTargetRel()].insert(rules_ptr[i].get());
            currID += 1;
            rules.push_back(std::move(rules_ptr[i]));
        }
    }
    std::cout<<"Loaded and indexed "<<currID<<" rules."<<std::endl;
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
        int numPreds = 100;
        int numTrue = 100;
        std::cout<<"Warning: could not find num preds and support for input line " + splitline[0]<<std::endl;
        std::cout<<" Setting both to 100. Expect random ordering for rules and predictions, confidence scores will all be 1."<<std::endl;
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

std::unordered_map<int, std::set<Rule*,compareRule>>& RuleStorage::getRelToRules(){
    return relToRules;
}

std::vector<std::unique_ptr<Rule>>& RuleStorage::getRules(){
    return rules;
 }

void RuleStorage::clearAll(){
    rules.clear();
    relToRules.clear();
}