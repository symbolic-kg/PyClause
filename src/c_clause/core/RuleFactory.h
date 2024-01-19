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
    int ZnumUnseen = 5;


    bool createRuleB = true;
    int BbranchingFactor = -1;
    int BnumUnseen = 5;

    bool createRuleC = true;
    int CnumUnseen = 5;

    bool createRuleD = true;
    double DconfWeight = 1.0;
    int DbranchingFactor = -1;
    int DnumUnseen = 5;


    bool createRuleXXd = true;
    int XXDnumUnseen = 5;

    bool createRuleXXc = true;
    int XXCnumUnseen = 5;

    int ZminCorrect = 2;
    int BminCorrect = 2;
    int CminCorrect = 2;
    int DminCorrect = 2;
    int XXDminCorrect = 2;
    int XXCminCorrect = 2;

    int ZminPreds = 2;
    int BminPreds = 2;
    int CminPreds = 2;
    int DminPreds = 2;
    int XXDminPreds = 2;
    int XXCminPreds = 2;

    int BmaxLength = -1;
    int CmaxLength = -1;
    int DmaxLength = -1;


    double ZminConf = 0.0001;
    double BminConf = 0.0001;
    double CminConf = 0.0001;
    double DminConf = 0.0001;
    double XXDminConf = 0.0001;
    double XXCminConf = 0.0001;
};


#endif