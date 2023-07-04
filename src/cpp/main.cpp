#include <iostream>
#include "myClass.h"

int main(){
    std::cout << "Hello, yes from rule_backend! \n";
    myClass ob; 
    long start = 1;
    long end = 100000;
    std::cout<<ob.addRange(start,end);
    return 0;
}
