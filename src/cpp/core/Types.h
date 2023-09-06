//adapted and inspired from https://github.com/OpenBioLink/SAFRAN/blob/master/include/Types.h

#ifndef TYPES_H
#define TYPES_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <array>

typedef std::unordered_set<int> Nodes;
typedef std::unordered_map<int, Nodes> NodeToNodes;
typedef std::unordered_map<int, NodeToNodes> RelNodeToNodes;

// [relation, head, tail]
typedef std::array<std::string,3> strAtom;

//used in parsing from string rules
struct symAtom {
    bool containsConstant;
    bool leftC; // assuming direction is represented as a string
    int constant;  // assuming constant is represented as a string
};

//used in internal processing
//[head, relation, tail]
typedef std::array<int, 3> Triple; 



class Rule;
// it needs to be ensured that rules are added in sorted order
typedef std::unordered_map<int, std::vector<Rule*>> NodeToPredRules;

typedef std::vector<std::pair<int, double>> CandidateConfs;


//typedef std::unordered_map<int, std::vector<Rule*>> RelToRules;

#endif //TYPES_H