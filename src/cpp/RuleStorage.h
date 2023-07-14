#ifndef RULESTORAGE_H
#define RULESTORAGE_H

#include "Rule.h"
#include "Types.h"
#include <vector>

class RuleStorage
{

public:
    RuleStorage(std::shared_ptr<Index> index);
    //if sampled the first 3 values in a ruleset refer to sampled values
    void readAnyTimeFormat(std::string path, bool sampled); 
    std::unique_ptr<Rule> parseAnytimeRule(std::string rule);
    std::vector<std::unique_ptr<Rule>>& getRules();

private:
    std::vector<std::unique_ptr<Rule>> rules;
    std::shared_ptr<Index> index;
};












#endif // RULESTORAGE_H