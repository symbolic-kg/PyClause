#include <chrono>
#include <iostream>
#include <string>
#include <memory>
#include "myClass.h"
#include "Index.h"
#include "TripleStorage.h"
#include "Rule.h"
#include "RuleStorage.h"

int main(){
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
    int rel1 = *(index->getIdOfRelationstring(rel1Str));
    int rel2 = *(index->getIdOfRelationstring(rel2Str));

    std::string c1str = "08638442";
    std::string c2str = "08524735";
    int c1 = *(index->getIdOfNodestring(c1str));
    int c2 = *(index->getIdOfNodestring(c2str));
    std::vector<int> relations = {rel1, rel2};
    std::vector<bool> directions = {true};
    bool leftC = false;
    std::array<int,2> constants = {c1, c2};

    RuleC rule(relations, directions, leftC, constants);

    auto pred = rule.materialize(data);

     // print predictions for rule
    int counter = 0;
    for (auto pred: rule.materialize(data)) {
        counter +=1;
        std::cout << "[";
        for (int i = 0; i < pred.size(); ++i) {
            std::cout << pred[i];
            if (i != pred.size() - 1) // not the last item
                std::cout << ", ";
        }
        std::cout << "]\n";
    }  
    std::cout<<"found:"<<counter<<"\n";

    // std::string rulePath = "/home/patrick/Desktop/PyClause/data/wnrr/rules-10";
    // RuleStorage rules(index);
    // rules.read(rulePath);
    
    std::cout<<"bye";

   
    return 0;
}
