#include <algorithm>

#include "Rule.h"
#include "Types.h"

// ***Base Rule implementation***

void Rule::setID(int ID){
    this->ID = ID;
}

void Rule::print(){
    throw std::runtime_error("Not implemented yet");
}

int Rule::getID(){
    return ID;
}

double Rule::getAppliedConfidence(int nUnseen){
    return (double) cpredicted/((double) predicted + (double)nUnseen); 
}

double Rule::getConfidence(){
    return (double) cpredicted/(double) predicted; 
}

std::string Rule::getRuleString(){
    throw std::runtime_error("Not implemented yet");
}
long long Rule::getBodyHash(){
    throw std::runtime_error("Not implemented yet");

}
void Rule::computeBodyHash(){
    throw std::runtime_error("Not implemented yet");

}
int Rule::getTargetRel(){
   return targetRel;

}
std::vector<std::vector<int>> Rule::materialize(TripleStorage& triples){
    throw std::runtime_error("Not implemented yet");
}

std::vector<int>& Rule::getRelations() {
    return relations;
}

std::vector<bool>& Rule::getDirections() {
    return directions;
}

bool Rule::predictHeadQuery(int head, TripleStorage& triples, NodeToPredRules& tailResults){
    throw std::runtime_error("Not implemented yet.");
}
bool Rule::predictTailQuery(int tail, TripleStorage& triples, NodeToPredRules& headResults){
    throw std::runtime_error("Not implemented yet.");
}

// ***RuleB implementation*** 

RuleB::RuleB(std::vector<int>& relations, std::vector<bool>& directions) {
    if(relations.size() != (directions.size() + 1)) {
        throw std::invalid_argument("'Directions' size should be one less than 'relations' size in construction of RuleB");
    }
    if(relations.size() < 2) {
        throw std::invalid_argument("Cannot construct a RuleB with no body atom.");
    }		
	this->relations = relations;
    this->directions = directions;	
    this->targetRel = relations.front();   

     // used for predicting heads
    this->_relations = relations;
    std::reverse(_relations.begin()+1, _relations.end());
    this->_directions = directions;
    std::reverse(_directions.begin(), _directions.end());
    _directions.flip();   
}

// TODO: dont return, just add predictions to a passed data structure
// or not return a copy
std::vector<std::vector<int>> RuleB::materialize(TripleStorage& triples){

    std::vector<std::vector<int>> predictions;

    RelNodeToNodes* relNtoN = nullptr;
     // first body atom is (v1,v2)
    if (directions[0]){
         relNtoN =  &triples.getRelHeadToTails();
    // first body atom is (v2,v1)    
    }else{
         relNtoN =  &triples.getRelTailToHeads();
    }
     // first body relation
    auto it = relNtoN->find(relations[1]);
    if (!(it==relNtoN->end())){
        NodeToNodes& NtoN = it->second;
         // start branches of the DFS search
         // every entity e that satisfies b1(e,someY) [or b1(someX, e)]
         for (auto const& pair: NtoN){
                const int& e = pair.first;
                Nodes closingEntities;
                std::set<int> substitutions = {e};
                searchCurrGroundings(1, e, substitutions, triples, closingEntities, relations, directions);
                for (const int& cEnt:  closingEntities){
                    std::vector<int> pred = {e, targetRel, cEnt};
                    predictions.push_back(pred);
                }
            }
    }
    return predictions;

}

bool RuleB::predictTailQuery(int head, TripleStorage& triples, NodeToPredRules& tailResults){
    RelNodeToNodes* relNtoN = nullptr;
    if (directions[0]){
        relNtoN =  &triples.getRelHeadToTails();   
    }else{
        relNtoN =  &triples.getRelTailToHeads();
    }
    auto it = relNtoN->find(relations[1]);
    if (it!=relNtoN->end()){
        NodeToNodes& NtoN = it->second;
        if (NtoN.count(head)>0){
            Nodes closingEntities;
            std::set<int> substitutions = {head};
            searchCurrGroundings(1, head, substitutions, triples, closingEntities, relations, directions);
            for (const int& cEnt: closingEntities){
                tailResults[cEnt].push_back(ID);
            }
            return !closingEntities.empty();
        }
    }
    return false;
}

bool RuleB::predictHeadQuery(int tail, TripleStorage& triples, NodeToPredRules& headResults){
    RelNodeToNodes* relNtoN = nullptr;
    if (_directions[0]){
        relNtoN =  &triples.getRelHeadToTails();   
    }else{
        relNtoN =  &triples.getRelTailToHeads();
    }
    auto it = relNtoN->find(_relations[1]);
    if (it!=relNtoN->end()){
        NodeToNodes& NtoN = it->second;
        if (NtoN.count(tail)>0){
            Nodes closingEntities;
            std::set<int> substitutions = {tail};
            searchCurrGroundings(1, tail, substitutions, triples, closingEntities, _relations, _directions);
            for (const int& cEnt: closingEntities){
                headResults[cEnt].push_back(ID);
            }
            return !closingEntities.empty();
        }
    }
    return false;
}



// recursive DFS from a startpoint currEntity, one substitution of the first body atom
void RuleB::searchCurrGroundings(
			int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
            Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs
		)
{
    Nodes* nextEntities = nullptr;
    int currRel = rels[currAtomIdx];
    RelNodeToNodes& relNtoN = dirs[currAtomIdx-1] ? triples.getRelHeadToTails() : triples.getRelTailToHeads();
    auto it = relNtoN.find(rels[currAtomIdx]);
    if (!(it==relNtoN.end())){
        NodeToNodes& NtoN = it->second;
        auto entIt = NtoN.find(currEntity);
        if (!(entIt==NtoN.end())){
            nextEntities = &(entIt->second);

            if (currAtomIdx == rels.size()-1){
                //copies
                for(const int ent: *nextEntities){
                    // respect object identity constraint, stop if violated
                    if (substitutions.find(ent)==substitutions.end()){
                        closingEntities.insert(ent);
                    }
                }
            }else{
                for(int ent: *nextEntities){
                    if (substitutions.find(ent)==substitutions.end()){
                        substitutions.insert(ent);
                        searchCurrGroundings(currAtomIdx+1, ent, substitutions, triples, closingEntities, rels, dirs);
                        substitutions.erase(ent);
                    }
                }
            }
        }
    } 
}

// ***RuleC implementation*** 

RuleC::RuleC(std::vector<int>& relations, std::vector<bool>& directions, bool& leftC, std::array<int,2>& constants) {
    if(relations.size() != (directions.size() + 1)) {
        throw std::invalid_argument("'Directions' size should be one less than 'relations' size in construction of RuleC");
    }
    if(relations.size() < 2) {
        throw std::invalid_argument("Cannot construct a RuleC with no body atom.");
    }		
	this->relations = relations;
    this->directions = directions;
    this->leftC = leftC;	
    this->constants = constants;
    this->targetRel = relations.front();

    // used for rules where leftC=false and predicting heads
    this->_relations = relations;
    std::reverse(_relations.begin()+1, _relations.end());
    this->_directions = directions;
    std::reverse(_directions.begin(), _directions.end());
    _directions.flip();   
}


// TODO: dont return, just add predictions to a passed data structure
// or not return a copy
std::vector<std::vector<int>> RuleC::materialize(TripleStorage& triples){

    std::vector<std::vector<int>> predictions;
    // if left head variable is grounded we start with with the first body atom it contains the second constant
    // if right is grounded we start with last body atom which then contains the second constant
    std::vector<int>& rels = leftC ? relations : _relations;
    std::vector<bool>& dirs = leftC ? directions: _directions;
    RelNodeToNodes* relNtoN = nullptr;
    if (dirs[0]){
        relNtoN =  &triples.getRelHeadToTails();   
    }else{
        relNtoN =  &triples.getRelTailToHeads();
    }
    auto it = relNtoN->find(rels[1]);
    if (!(it==relNtoN->end())){
        NodeToNodes& NtoN = it->second;
        if (NtoN.count(constants[1])>0){
            Nodes closingEntities;
            // we enforce OI for both the constants, this is consistent with B rules
            std::set<int> substitutions = {constants[0], constants[1]};
            searchCurrGroundings(1, constants[1], substitutions, triples, closingEntities, rels, dirs);
            for (const int& cEnt:  closingEntities){
                if (leftC){
                    predictions.push_back({constants[0], targetRel, cEnt});
                }else{
                    predictions.push_back({cEnt, targetRel, constants[0]});
                }
            }
            return predictions;
        }
    }
}

bool RuleC::predictTailQuery(int head, TripleStorage& triples, NodeToPredRules& tailResults){
    // can only predict my constant in the grounded direction
    if (leftC && head!=constants[0]){
        return false;
    }

    std::vector<int>& rels = leftC ? relations : _relations;
    std::vector<bool>& dirs = leftC ? directions: _directions;
    RelNodeToNodes* relNtoN = nullptr;
    if (dirs[0]){
        relNtoN =  &triples.getRelHeadToTails();   
    }else{
        relNtoN =  &triples.getRelTailToHeads();
    }
    auto it = relNtoN->find(rels[1]);
    if (it!=relNtoN->end()){
        NodeToNodes& NtoN = it->second;
        if (NtoN.count(constants[1])>0){
            Nodes closingEntities;
            std::set<int> substitutions = {constants[0], constants[1]};
            searchCurrGroundings(1, constants[1], substitutions, triples, closingEntities, rels, dirs);
            bool madePred = false;
            for (const int& cEnt: closingEntities){
                // the rule is grounded at the tail so it can only predict this grounding
                // and the closing entity must be the head grounding in this case
                if (!leftC && cEnt == head){
                    tailResults[constants[0]].push_back(ID);
                    return true;
                }else if(leftC){
                    tailResults[cEnt].push_back(ID);
                    madePred = true;
                }
            }
            return madePred;
        }
    }
    return false;
}

bool RuleC::predictHeadQuery(int tail, TripleStorage& triples, NodeToPredRules& headResults){
    // can only predict my constant in the grounded direction
    if (!leftC && tail!=constants[0]){
        return false;
    }

    std::vector<int>& rels = leftC ? relations : _relations;
    std::vector<bool>& dirs = leftC ? directions: _directions;
    RelNodeToNodes* relNtoN = nullptr;
    if (dirs[0]){
        relNtoN =  &triples.getRelHeadToTails();   
    }else{
        relNtoN =  &triples.getRelTailToHeads();
    }
    auto it = relNtoN->find(rels[1]);
    if (it!=relNtoN->end()){
        NodeToNodes& NtoN = it->second;
        if (NtoN.count(constants[1])>0){
            Nodes closingEntities;
            std::set<int> substitutions = {constants[0], constants[1]};
            searchCurrGroundings(1, constants[1], substitutions, triples, closingEntities, rels, dirs);
            bool madePred = false;
            for (const int& cEnt: closingEntities){
                // the rule is grounded at the head so it can only predict this grounding
                // and the closing entity must be the tail grounding in this case
                if (leftC && cEnt == tail){
                    headResults[constants[0]].push_back(ID);
                    return true;
                }else if(!leftC){
                    headResults[cEnt].push_back(ID);
                    madePred = true;
                }
            }
            return madePred;
        }
    }
    return false;
}

// same implementation as in RuleB except that rels dirs  is used depending on leftC
// directions can be handled
void RuleC::searchCurrGroundings(
			int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
            Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs
		)
{
    Nodes* nextEntities = nullptr;
    int currRel = rels[currAtomIdx];
    RelNodeToNodes& relNtoN = dirs[currAtomIdx-1] ? triples.getRelHeadToTails() : triples.getRelTailToHeads();
    auto it = relNtoN.find(rels[currAtomIdx]);
    if (!(it==relNtoN.end())){
        NodeToNodes& NtoN = it->second;
        auto entIt = NtoN.find(currEntity);
        if (!(entIt==NtoN.end())){
            nextEntities = &(entIt->second);

            if (currAtomIdx == rels.size()-1){
                //copies
                for(const int& ent: *nextEntities){
                    // respect object identity constraint
                    if (substitutions.find(ent)==substitutions.end()){
                        closingEntities.insert(ent);
                    }
                }
            }else{
                for(const int& ent: *nextEntities){
                    if (substitutions.find(ent)==substitutions.end()){
                        substitutions.insert(ent);
                        searchCurrGroundings(currAtomIdx+1, ent, substitutions, triples, closingEntities, rels, dirs);
                        substitutions.erase(ent);
                    }
                }
            }
        }
    } 
}