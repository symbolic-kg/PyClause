#ifndef RULESTORAGE_H
#define RULESTORAGE_H

#include "Rule.h"
#include "Globals.h"
#include "Types.h"

#include <vector>


struct compareRule {
    bool operator () (Rule* lhs, Rule* rhs) const {
        return lhs->getConfidence(_cfg_nUnseen, _cfg_exactConf) > rhs->getConfidence(_cfg_nUnseen, _cfg_exactConf);
    }
};


class RuleStorage
{

public:
    RuleStorage(std::shared_ptr<Index> index);
    // expects a file with lines val\tval\t\val\trulestring
    //if sampled the first 3 values in a ruleset refer to sampled values
    void readAnyTimeFormat(std::string path, bool sampled); 
    std::unique_ptr<Rule> parseAnytimeRule(std::string rule);
    std::vector<std::unique_ptr<Rule>>& getRules();
    std::set<Rule*, compareRule>& getRelRules(int relation);

    // add a rule to the storage
    // this should be the only one who adds rules to the storage
    // takes care of individual rule options/ ignoring certain rule types etc..
    // also assigns the rule id's
    void addRule(std::unique_ptr<Rule*> rule);



    void parseAtom(const std::string& input, strAtom& atom);
    void parseSymAtom(strAtom& inputAtom, symAtom& symA);

private:
    // rules owns the rule objects
    // TODO unclear atm if we need both rules and RelToRules, but it seems useful to access rules globally by index
    // RelToRules keeps the rules sorted due to the set, iterating over all the rules would
    // be faster with a vector, though
    // accessing rules by relation and index needs to be hacked also, let's see
    std::vector<std::unique_ptr<Rule>> rules;
    std::unordered_map<int, std::set<Rule*,compareRule>> relToRules;
    std::shared_ptr<Index> index;
    

};

#endif // RULESTORAGE_H