#ifndef RULESTORAGE_H
#define RULESTORAGE_H

#include "Rule.h"
#include "Types.h"
#include <vector>

class RuleStorage
{

public:
    RuleStorage(std::shared_ptr<Index> index);
    // expects a file with lines val\tval\t\val\trulestring
    //if sampled the first 3 values in a ruleset refer to sampled values
    void readAnyTimeFormat(std::string path, bool sampled); 
    std::unique_ptr<Rule> parseAnytimeRule(std::string rule);
    std::vector<std::unique_ptr<Rule>>& getRules();
    void parseAtom(const std::string& input, strAtom& atom);
    void parseSymAtom(strAtom& inputAtom, symAtom& symA);

private:
    std::vector<std::unique_ptr<Rule>> rules;
    std::shared_ptr<Index> index;
};












#endif // RULESTORAGE_H