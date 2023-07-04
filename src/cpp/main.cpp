#include <iostream>
#include <string>
#include "myClass.h"

int main(){
    std::cout << "Hello, yesoo from rule_backend! \n";
    myClass ob("peter"); 
    long start = 1;
    long end = 100000;
    std::cout<<ob.addRange(start,end);
    return 0;
}
