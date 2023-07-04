#include <string>
class myClass
{
public:
    int number;
    std::string name;

    myClass(std::string name_);
    void addOne();

    int getNumber();
    long addRange(long start, long end);
    
};