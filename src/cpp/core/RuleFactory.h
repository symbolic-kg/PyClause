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

    std::unique_ptr<Rule>parseUXXrule(std::vector<std::string> headBody);

    void parseAtom(const std::string& input, strAtom& atom);
    void parseSymAtom(strAtom& inputAtom, symAtom& symA);

   
    void setCreateRuleB(bool ind);
    void setCreateRuleC(bool ind);
    void setCreateRuleZ(bool ind);
    void setCreateRuleD(bool ind);
    void setCreateRuleXXd(bool ind);
    void setCreateRuleXXc(bool ind);
    void setNumUnseen(int val);

private:
    std::shared_ptr<Index> index;

    // ***Rule options***
    bool createRuleZ = true;
    bool createRuleB = true;
    bool createRuleC = true;
    bool createRuleD = true;
    bool createRuleXXd = true;
    bool createRuleXXc = true;

    // laplace smoothing for rule confidence
    int num_unseen = 5;
};


#endif