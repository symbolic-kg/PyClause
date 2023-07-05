#include <iostream>
#include <string>
#include "myClass.h"
#include "Index.h"
#include "TripleStorage.h"

int main(){
    std::cout << "Hello, yesoo from rule_backend! \n";
    myClass ob("peter"); 
    long start = 1;
    long end = 100000;
    std::cout<<ob.addRange(start,end)<<"\n";


    Index* index = new Index();
    std::string dataPath = "/home/patrick/Desktop/PyClause/data/wnrr/train.txt";
    TripleStorage* data = new TripleStorage(dataPath, index);
    std::cout<<"data loaded";




    
    return 0;
}
