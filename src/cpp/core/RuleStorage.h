#ifndef RULESTORAGE_H
#define RULESTORAGE_H

#include "Rule.h"
#include "Globals.h"
#include "Types.h"
#include "RuleFactory.h"

#include <vector>


struct compareRule {
    bool operator () (Rule* lhs, Rule* rhs) const {
        double lconf =  lhs->getConfidence(_cfg_exactConf);
        double rconf = rhs->getConfidence(_cfg_exactConf);
        if (lconf != rconf){
            return lconf>rconf;
        }
        else{
            return true;
        }
    }
};


class RuleStorage
{

public:
    RuleStorage(std::shared_ptr<Index> index, std::shared_ptr<RuleFactory> ruleFactory);
    // expects a file with lines val\tval\t\val\trulestring
    //if sampled the first 3 values in a ruleset refer to sampled values
    void readAnyTimeFormat(std::string path, bool sampled); 
    std::vector<std::unique_ptr<Rule>>& getRules();
    std::set<Rule*, compareRule>& getRelRules(int relation);

    void addRule(std::unique_ptr<Rule*> rule);
    

private:
    // rules owns the rule objects
    // TODO unclear atm if we need both rules and RelToRules, but it seems useful to access rules globally by index
    // RelToRules keeps the rules sorted due to the set, iterating over all the rules would
    // be faster with a vector, though
    // accessing rules by relation and index needs to be hacked also, let's see
    std::vector<std::unique_ptr<Rule>> rules;
    std::unordered_map<int, std::set<Rule*,compareRule>> relToRules;
    std::shared_ptr<Index> index;
    // TODO you dont really need a shared pointer here; but at least options should be global 
    std::shared_ptr<RuleFactory> ruleFactory;

    bool verbose = true;
    

};

#endif // RULESTORAGE_H