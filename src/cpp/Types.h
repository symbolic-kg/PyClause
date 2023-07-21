//adapted and inspired from https://github.com/OpenBioLink/SAFRAN/blob/master/include/Types.h

#ifndef TYPES_H
#define TYPES_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>

//#include "Rule.h"

typedef std::unordered_set<int> Nodes;
typedef std::unordered_map<int, Nodes> NodeToNodes;
typedef std::unordered_map<int, NodeToNodes> RelNodeToNodes;

// [relation, head, tail]
typedef std::array<std::string,3> strAtom;

struct symAtom {
    bool containsConstant;
    bool leftC; // assuming direction is represented as a string
    int constant;  // assuming constant is represented as a string
};

typedef std::unordered_map<int, std::vector<int>> NodeToPredRules;


//typedef std::unordered_map<int, std::vector<Rule*>> RelToRules;

#endif //TYPES_H