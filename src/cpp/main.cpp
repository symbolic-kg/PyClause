#include <iostream>
#include <string>
#include <memory>
#include <omp.h>

#include "myClass.h"
#include "Index.h"
#include "TripleStorage.h"
#include "Rule.h"
#include "RuleStorage.h"

void tests(){
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::string dataPath = "/home/patrick/Desktop/kge/data/wnrr/train.txt";
    TripleStorage data(index);
    data.read(dataPath);

    RuleStorage rules(index);
    std::unique_ptr<Rule> ruleB = rules.parseAnytimeRule("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)");
    size_t size = (ruleB->materialize(data)).size();
    if (!size==83){
        throw std::runtime_error("Test 1 for B-rule failed.");
    }

    
    // Test num predictions
    //276	3	0.010869565217391304	_hypernym(X,00732746) <= _synset_domain_topic_of(X,A), _derivationally_related_form(A,10225219)
    std::unique_ptr<Rule> ruleC = rules.parseAnytimeRule("_hypernym(X,00732746) <= _synset_domain_topic_of(X,A), _derivationally_related_form(A,10225219)");
    
    // code for casting to RuleC class
    // RuleC* ruleCPtr = dynamic_cast<RuleC*>(ruleC.get());
    // ruleC.release(); // Release old pointer
    // std::unique_ptr<RuleC> ruleCC(ruleCPtr); // Create new unique_ptr
    size = (ruleC->materialize(data)).size();
    if (!size==276){
        throw std::runtime_error("Test 1 for C-rule failed.");
    }
    //Test num predictions
    // 4	2	0.5	_derivationally_related_form(00748155,Y) <= _derivationally_related_form(A,Y), _derivationally_related_form(00748155,A)
    ruleC = rules.parseAnytimeRule("_derivationally_related_form(00748155,Y) <= _derivationally_related_form(A,Y), _derivationally_related_form(00748155,A)");
    size = (ruleC->materialize(data)).size();
    if (!size==4){
        throw std::runtime_error("Test 2 for C-rule failed.");
    }

    // flip direction of both atoms
    ruleC = rules.parseAnytimeRule("_hypernym(X,00732746) <= _synset_domain_topic_of(A,X), _derivationally_related_form(10225219,A)");
    if (ruleC->getDirections()[0] || ruleC->getDirections()[1]){
        throw std::runtime_error("Test 3 for C-rule failed");
    }

    // flip direction of both atoms
    ruleC = rules.parseAnytimeRule("_derivationally_related_form(00748155,Y) <= _derivationally_related_form(Y,A), _derivationally_related_form(A,00748155)");
    if (ruleC->getDirections()[0] || ruleC->getDirections()[1]){
        throw std::runtime_error("Test 4 for C-rule failed");
    }

    //Test num predictions
    // 59	11	0.1864406779661017	_member_meronym(12998349,Y) <= _hypernym(Y,11590783)
    ruleC = rules.parseAnytimeRule("_member_meronym(12998349,Y) <= _hypernym(Y,11590783)");
    size = (ruleC->materialize(data)).size();
    if (!size==59){
        throw std::runtime_error("Test 5 for C-rule failed.");
    }

    //Test num predictions
    //162	2	0.012345679012345678	_hypernym(X,11669921) <= _member_meronym(11911591,X)
    ruleC = rules.parseAnytimeRule("_hypernym(X,11669921) <= _member_meronym(11911591,X)");
    size = (ruleC->materialize(data)).size();
    if (!size==162){
        throw std::runtime_error("Test 6 for C-rule failed.");
    }

    std::cout<<"All tests passed."<<std::endl;
}

int main(){
    tests();
    std::cout << "Hello, yesoo from rule_backend! \n";
    myClass ob("peter"); 
    long start = 1;
    long end = 100000;
    std::cout<<ob.addRange(start,end)<<"\n";

    // data loading
    
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::string dataPath = "/home/patrick/Desktop/kge/data/wnrr/train.txt";
    TripleStorage data(index);
    data.read(dataPath);
    std::cout<<"data loaded \n";
    std::cout<<"bye \n";

    // rule example
    //std::vector<int> relations = {1,6,4};
    //std::vector<bool> directions = {true,true};

    //RuleB rule(relations, directions);
    //std::cout<<rule.getTargetRel()<<"\n";

    Rule empty;
    std::cout<<empty.getTargetRel()<<"\n";

    // real C-rule
    std::string rel1Str = "_instance_hypernym";
    std::string rel2Str = "_instance_hypernym";
    int rel1 = index->getIdOfRelationstring(rel1Str);
    int rel2 = index->getIdOfRelationstring(rel2Str);

    std::string c1str = "08638442";
    std::string c2str = "08524735";
    int c1 = index->getIdOfNodestring(c1str);
    int c2 = index->getIdOfNodestring(c2str);
    std::vector<int> relations = {rel1, rel2};
    std::vector<bool> directions = {true};
    bool leftC = false;
    std::array<int,2> constants = {c1, c2};

    RuleC rule(relations, directions, leftC, constants);


    //25 predictions? :"_has_part(X,Y) <= _has_part(A,X), _member_meronym(A,B), _derivationally_related_form(B,C), _derivationally_related_form(C,D), _has_part(D,Y)"
    // parse a real B rule and materialize

    //83 30 rule correct preds from christian _has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)
    RuleStorage rules(index);
    std::unique_ptr<Rule> ruleB = rules.parseAnytimeRule("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)");
    
    strAtom atom;
    std::string input = "rel1(X,Y)";
    rules.parseAtom(input, atom);


    std::cout<<"im here";
    //rules.parseAnytimeRule("_has_part(X,08638442) <= _has_part(A,X), _has_part(A,B), _has_part(B,08638442)");
    // std::unique_ptr<Rule> ruleC = rules.parseAnytimeRule("_has_part(X,08638442) <= _has_part(A,X), _has_part(A,B), _has_part(B,08638442)");
    // std::vector<bool> dirsC = ruleC->getDirections();
    // std::vector<int> relsC = ruleC->getRelations();

     // print predictions for rule
    int counter = 0;
    for (auto pred: ruleB->materialize(data)) {
        counter +=1;
        std::cout << "[";
        for (int i = 0; i < pred.size(); ++i) {
            if (i==0 | i==2){
                std::cout << index->getStringOfNodeId(pred[i]);
            }else{
                std::cout << index->getStringOfRelId(pred[i]);
            }
            if (i != pred.size() - 1) // not the last item
                std::cout << ", ";
        }
        std::cout << "]\n";
    }  
    std::cout<<"found:"<<counter<<"\n";

    //**** read and materialize rules ***
    std::string rulePath = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600";
    rules.readAnyTimeFormat(rulePath, true); 
    std::vector<std::unique_ptr<Rule>>& allRules = rules.getRules();
    // int ctr = 0;

    // // for (auto& srule: allRules){
    // //     srule->materialize(data);
    // //     std::cout<<"materialized rule:"<<ctr<<"\n";
    // //     ctr+=1;
    // // }


    #pragma omp parallel
    {
    #pragma omp for
    for (int i = 0; i < allRules.size(); ++i){
        allRules[i]->materialize(data);
        std::cout<<"materialize rule:"<< i <<"\n";
    }
    }


    std::cout<<"bye";

   
    return 0;
}
