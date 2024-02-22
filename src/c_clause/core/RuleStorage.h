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

    void readAnyTimeParFormat(std::string path, bool exact, int numThreads);

    bool addAnyTimeRuleWithStats(std::string ruleString, int id, int numPred, int numTrue, bool exact);
    std::vector<std::unique_ptr<Rule>>& getRules();
    std::unordered_map<int, std::set<Rule*,compareRule>>& getRelToRules();
    std::set<Rule*, compareRule>& getRelRules(int relation);
    void clearAll();
    

private:
    // rules owns the rule objects
    // RelToRules keeps the rules sorted due to the set, iterating over all the rules would
    // rules defines the global index but should never be used for application features
    // e.g. from python side user may want to subset rules which will only affect relToRules
    // as we need to be able to come back to larger sets after subsetting
    std::vector<std::unique_ptr<Rule>> rules;
    // from here rule application is performed; application is always based on a target relation
    std::unordered_map<int, std::set<Rule*,compareRule>> relToRules;
    std::shared_ptr<Index> index;
    // TODO you dont really need a shared pointer here; but at least options should be global 
    std::shared_ptr<RuleFactory> ruleFactory;

    bool verbose = true;
    

};

#endif // RULESTORAGE_H