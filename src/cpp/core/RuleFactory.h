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

private:
    std::shared_ptr<Index> index;
    void setRuleOptions(Rule& rule);


    // **individual rule options**
    double ruleZweight = 1.0;
    double ruleBweight = 1.0;



};


#endif