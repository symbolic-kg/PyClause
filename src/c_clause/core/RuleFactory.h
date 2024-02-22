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
    // if numPreds and numTrue given it will also check if the rule should based on the member constraints
    // e.g. BminNumPreds
    std::unique_ptr<Rule> parseAnytimeRule(std::string rule, int numPreds=-1, int numTrue=-1);

    std::unique_ptr<Rule> parseUXXrule(std::vector<std::string> headBody, int numPreds=-1, int numTrue=-1);
    std::unique_ptr<Rule> parseUcRule(std::vector<std::string> headBody, int numPreds=-1, int numTrue=-1);
    std::unique_ptr<Rule> parseUdRule(std::vector<std::string> headBody, int numPreds=-1, int numTrue=-1);

    void parseAtom(const std::string& input, strAtom& atom);
    void parseSymAtom(strAtom& inputAtom, symAtom& symA);

    // updates relToRules based on the set rule options and rules; rules is unchanged
    void updateRules(std::vector<std::unique_ptr<Rule>>& rules, std::unordered_map<int, std::set<Rule*,compareRule>>& relToRules);

    
    void setCreateRuleB(bool ind);
    void setCreateRuleC(bool ind);
    void setCreateRuleZ(bool ind);
    void setCreateRuleD(bool ind);
    void setCreateRuleXXd(bool ind);
    void setCreateRuleXXc(bool ind);
    void setNumUnseen(int val, std::string type);

    void setMinCorrect(int val, std::string type);
    void setMinPred(int val, std::string type);
    void setMinConf(double val, std::string type);

    void setBbranchingFactor(int val);
    void setDbranchingFactor(int val);
    void setDconfWeight(double val);
    void setZconfWeight(double val);

    void setBmaxLength(int val);
    void setCmaxLength(int val);
    void setDmaxLength(int val);

private:
    std::shared_ptr<Index> index;

    // ***Rule options***
    bool createRuleZ = true;
    double ZconfWeight = 1.0;
    int ZnumUnseen = 0;


    bool createRuleB = true;
    int BbranchingFactor = -1;
    int BnumUnseen = 0;

    bool createRuleC = true;
    int CnumUnseen = 0;

    bool createRuleD = true;
    double DconfWeight = 1.0;
    int DbranchingFactor = -1;
    int DnumUnseen = 0;


    bool createRuleXXd = true;
    int XXDnumUnseen = 0;

    bool createRuleXXc = true;
    int XXCnumUnseen = 0;

    int ZminCorrect = 1;
    int BminCorrect = 1;
    int CminCorrect = 1;
    int DminCorrect = 1;
    int XXDminCorrect = 1;
    int XXCminCorrect = 1;

    int ZminPreds = 1;
    int BminPreds = 1;
    int CminPreds = 1;
    int DminPreds = 1;
    int XXDminPreds = 1;
    int XXCminPreds = 1;

    int BmaxLength = -1;
    int CmaxLength = -1;
    int DmaxLength = -1;


    double ZminConf = 0.00001;
    double BminConf = 0.00001;
    double CminConf = 0.00001;
    double DminConf = 0.00001;
    double XXDminConf = 0.00001;
    double XXCminConf = 0.00001;
};


#endif