#ifndef MULTISET_H
#define MULTISET_H

#include <vector>
#include <unordered_set>

class ManySet {
    private:
        std::vector<const std::unordered_set<int>*> sets;

    public:
        void addSet(const std::unordered_set<int>* set) {
            sets.push_back(set);
        }
        
        bool contains(int input) {
            for(auto& set : sets) {
                if(set->find(input) != set->end()) {
                    return true;
                }
            }
            return false;
        }
};

#endif //MULTISET_H