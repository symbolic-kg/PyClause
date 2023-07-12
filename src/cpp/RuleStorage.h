#ifndef RULESTORAGE_H
#define RULESTORAGE_H

#include "Rule.h"
#include "Types.h"
#include <vector>

class RuleStorage
{

public:
    RuleStorage(std::shared_ptr<Index> index);
    void read(std::string path);

private:
    std::vector<std::unique_ptr<Rule>> rules;
    std::shared_ptr<Index> index;
};












#endif // RULESTORAGE_H