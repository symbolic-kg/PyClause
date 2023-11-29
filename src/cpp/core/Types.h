//adapted and inspired from https://github.com/OpenBioLink/SAFRAN/blob/master/include/Types.h

#ifndef TYPES_H
#define TYPES_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <array>
#include <string>
#include <map>


#include "Globals.h"


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
typedef std::vector<Triple> TripleSet;
typedef std::vector<std::array<std::string, 3>> StringTripleSet;

namespace std {
    template<> struct hash<Triple> {
        std::size_t operator()(const Triple& triple) const {
            std::size_t h1 = std::hash<int>{}(triple[0]);
            std::size_t h2 = std::hash<int>{}(triple[1]);
            std::size_t h3 = std::hash<int>{}(triple[2]);
            return h1 ^ (h2 << 1) ^ (h3 << 2); 
        }
    };
    template<> struct hash<array<string, 3>> {
        size_t operator()(const array<string, 3>& arr) const {
            size_t combined_hash = 0;
            hash<string> string_hasher;
            for (const auto& element : arr) {
                combined_hash ^= string_hasher(element) + 0x9e3779b9 + (combined_hash << 6) + (combined_hash >> 2);
            }
            return combined_hash;
        }
    };
}



class Rule;


// implemented in rule class to avoid circular includes
struct compareRule {
    bool operator()(Rule* lhs, Rule* rhs) const;
};

typedef std::map<Rule*, std::vector<std::vector<Triple>>, compareRule> RuleGroundings;

// it needs to be ensured that rules are added in sorted order
typedef std::unordered_map<int, std::vector<Rule*>> NodeToPredRules;

typedef std::vector<std::pair<int, double>> CandidateConfs;

//typedef std::unordered_map<int, std::vector<Rule*>> RelToRules;

#endif //TYPES_H