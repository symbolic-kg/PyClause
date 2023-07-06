#include <iostream>
#include <string>
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
    {
    Index* index = new Index();
    std::string dataPath = "/home/patrick/Desktop/kge/data/wikidata5m/train.txt";
    TripleStorage data(index);
    data.read(dataPath);
    } // destructor is called
    std::cout<<"data loaded \n";
    std::cout<<"bye";

    // rule example
    std::vector<int> relations = {1, 2, 3};
    std::vector<bool> directions = {true, true};
    RuleB rule(relations, directions);
    std::cout<<"bye";
    return 0;
}
