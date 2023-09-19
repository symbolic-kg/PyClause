#ifndef RULEFACTORY_H
#define RULEFACTORY_H


#include "Rule.h"
#include "Index.h"

#include <memory>
#include <string>





class RuleFactory
{
public:
    RuleFactory(std::shared_ptr<Index> index);
    std::unique_ptr<Rule> parseAnytimeRule(std::string rule);

    void parseAtom(const std::string& input, strAtom& atom);
    void parseSymAtom(strAtom& inputAtom, symAtom& symA);

   
    void setCreateRuleB(bool ind);
    void setCreateRuleC(bool ind);
    void setCreateRuleZ(bool ind);

private:
    std::shared_ptr<Index> index;
    bool createRuleZ = true;
    bool createRuleB = true;
    bool createRuleC = true;
};


#endif