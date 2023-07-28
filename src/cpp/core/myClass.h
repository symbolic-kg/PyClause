#ifndef MYCLASS_H
#define MYCLASS_H

#include <string>
class myClass
{
public:
    int number;
    std::string name;

    myClass(std::string name_){
    number = 1;
    name = name_;
}
    void addOne();

    int getNumber();
    long addRange(long start, long end);
    
};

#endif