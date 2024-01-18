#ifndef RULESTORAGE_H
#define RULESTORAGE_H

#include "Rule.h"
#include "Globals.h"
#include "Types.h"
#include "RuleFactory.h"

#include <vector>

class RuleStorage
{

public:
    RuleStorage(std::shared_ptr<Index> index, std::shared_ptr<RuleFactory> ruleFactory);
    // expects a file with lines val\tval\t\val\trulestring
    //if sampled the first 3 values in a ruleset refer to sampled values
    void readAnyTimeFormat(std::string path, bool exact); 
    void readAnyTimeFromVec(std::vector<std::string>& stringLines, bool exact);
    void readAnyTimeFromVecs(std::vector<std::string>& ruleStrings, std::vector<std::pair<int,int>> stats, bool exact); 

    // ruleLine is num_pred /t support /t conf /t ruleString
    bool addAnyTimeRuleLine(std::string ruleLine, int id, bool exact);

    bool addAnyTimeRuleWithStats(std::string ruleString, int id, int numPred, int numTrue, bool exact);
    std::vector<std::unique_ptr<Rule>>& getRules();
    std::set<Rule*, compareRule>& getRelRules(int relation);
    void clearAll();
    

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