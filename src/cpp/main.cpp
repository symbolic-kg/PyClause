#include <iostream>
#include <string>
#include <memory>
#include "myClass.h"
#include "Index.h"
#include "TripleStorage.h"
#include "Rule.h"

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
    std::vector<int> relations = {1,6};
    std::vector<bool> directions = {true};

    RuleB rule(relations, directions);
    std::cout<<rule.getTargetRel()<<"\n";

    Rule empty;
    std::cout<<empty.getTargetRel()<<"\n";

   for (auto pred: rule.materialize(data)) {
    std::cout << "[";
    for (int i = 0; i < pred.size(); ++i) {
        std::cout << pred[i];
        if (i != pred.size() - 1) // not the last item
            std::cout << ", ";
    }
    std::cout << "]\n";
}
    
    std::cout<<"bye";
    return 0;
}
