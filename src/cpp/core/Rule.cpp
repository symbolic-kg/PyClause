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

void Rule::setRuleString(std::string str){
    this->rulestring = str;
}

double Rule::getConfidence(int nUnseen, bool exact){
    if (exact){
        return confWeight * ((double) cpredicted/((double) predicted + (double)nUnseen)); 
    }else{
        return confWeight *((double) sampledCpredicted/((double) sampledPredicted + (double)nUnseen)); 
    }
    
}

void Rule::setStats(int _predicted, int _cpredicted, bool exact){
    if (exact){
        cpredicted = _cpredicted;
        predicted = _predicted;
    }else{
        sampledCpredicted = _cpredicted;
        sampledPredicted = _predicted;
    }
}

std::array<int,2> Rule::getStats(bool exact){
    if (exact){
        return {predicted, cpredicted};
    }else{
        return {sampledPredicted, sampledCpredicted};
    }
}

void Rule::setTrackInMaterialize(bool val){
    trackInMaterialize = val;
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
std::set<Triple> Rule::materialize(TripleStorage& triples){
    throw std::runtime_error("Not implemented yet");
}

std::vector<int>& Rule::getRelations() {
    return relations;
}

std::vector<bool>& Rule::getDirections() {
    return directions;
}

bool Rule::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
    throw std::runtime_error("Not implemented yet.");
}
bool Rule::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults,  ManySet filterSet){
    throw std::runtime_error("Not implemented yet.");
}

void Rule::setConfWeight(double weight){
    confWeight = weight;
}


// ***RuleB implementation*** 

int RuleB::branchingFaktor=-1;
int RuleB::discriminationBound=-1;

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


std::set<Triple> RuleB::materialize(TripleStorage& triples){

    std::set<Triple> predictions;

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
                    Triple triple = {e, targetRel, cEnt};
                    auto isNew = predictions.insert(triple);
                    // add to count if this triple is predicted for the first time
                    if (trackInMaterialize && isNew.second){
                        predicted+=1;
                        if (triples.contains(triple[0], triple[1], triple[2])){
                            cpredicted += 1;
                        }
                    }
                }
            }
    }
    return predictions;

}

bool RuleB::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
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
            bool madePred = false;
            for (const int& cEnt: closingEntities){ 
                if (!filterSet.contains(cEnt)){
                    tailResults.insertRule(cEnt, this);
                    madePred = true;
                }
                
            }
            return madePred;
        }
    }
    return false;
}

bool RuleB::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
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
            bool madePred = false;
            for (const int& cEnt: closingEntities){
                if (!filterSet.contains(cEnt)){
                    headResults.insertRule(cEnt, this);
                    madePred = true;
                }
                
            }
            return madePred;
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
        if (entIt!=NtoN.end()){
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
                // skip if branching factor is used and if exceeded
                if (RuleB::branchingFaktor>0 && nextEntities->size()>RuleB::branchingFaktor){
                    return;
                }
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
//DFS search where the starting point is the grounded body atom
// in the rule representation here this is the last body atom if leftC=false and first body atom if leftC=true
std::set<Triple> RuleC::materialize(TripleStorage& triples){

    std::set<Triple> predictions;
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
    if (it!=relNtoN->end()){
        NodeToNodes& NtoN = it->second;
        if (NtoN.count(constants[1])>0){
            Nodes closingEntities;
            // we enforce OI for both the constants, this is consistent with B rules
            std::set<int> substitutions = {constants[0], constants[1]};
            searchCurrGroundings(1, constants[1], substitutions, triples, closingEntities, rels, dirs);
            for (const int& cEnt:  closingEntities){
                std::pair<std::set<Triple>::iterator, bool> isNew;
                Triple triple;
                if (leftC){
                    triple = {constants[0], targetRel, cEnt};
                    isNew = predictions.insert(triple);
                }else{
                    triple = {cEnt, targetRel, constants[0]};
                    isNew = predictions.insert(triple);
                }
                // if triple is predicted for the first time track stats
                if (trackInMaterialize && isNew.second){
                    predicted += 1;
                    if (triples.contains(triple[0], triple[1], triple[2])){
                        cpredicted += 1;
                    }

                }
            }
            return predictions;
        }
    }
}

// contrary to B rules we let the DFS run starting from the grounded constants of the rules body
// and not from the grounded entity in the query 
bool RuleC::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    // can only predict my constant in the grounded direction
    if (leftC && head!=constants[0]){
        return false;
    }

    if (directions.size()==1){
        return predictL1TailQuery(head, triples, tailResults, filterSet);
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
                if (!leftC && cEnt == head && !filterSet.contains(constants[0])){
                    tailResults.insertRule(constants[0], this);
                    return true;
                }else if(leftC && !filterSet.contains(cEnt)){
                    tailResults.insertRule(cEnt, this);
                    madePred = true;
                }
            }
            return madePred;
        }
    }
    return false;
}


bool RuleC::predictL1TailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
    RelationalCSR* csr = triples.getCSR();

    // h(X,c) <-- b1(d,X) or h(X,c) <-- b1(X,d)
    // we want to only look up the body for X=head by looking from X=head to d and then predict c
    if (!leftC){

        if (filterSet.contains(constants[0])){
            return false;
        }
        int* begin;
        int length;
        int bodyRel = this->relations[1];
        directions[0] ? csr->getTforHREfficient(head, bodyRel, begin, length) :  csr->getHforTRFast(head, bodyRel, begin, length);
        std::cout<<length<<std::endl;
        for (int i=0; i<length; i++){
            if (begin[i]==constants[1]){
                tailResults.insertRule(constants[0], this);
                return true;

            }
        }
    
    }else{
         // h(c,Y) <-- b1(d,Y) or h(c,Y) <-- b1(Y,d) , predict all Y=y that ground the body
        int* begin;
        int length;
        int bodyRel = this->relations[1];
        directions[0] ? csr->getTforHREfficient(constants[1], bodyRel, begin, length) :  csr->getHforTRFast(constants[1], bodyRel, begin, length);
        bool predicted = false;
        for (int i=0; i<length; i++){
            int cand = begin[i];
            if (!filterSet.contains(cand)){
                tailResults.insertRule(cand, this);
                predicted = true;
            }
        }
        return predicted;
    }
    return false;

}


bool RuleC::predictL1HeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
    if (leftC){
        // h(c,Y) <-- b1(d,Y) or h(c,Y) <-- b1(Y,d)
        // we want to only look up the body for Y=tail by looking from Y=tail to d and then predict c
        RelNodeToNodes& relNtoN = directions[0] ? triples.getRelTailToHeads() :  triples.getRelHeadToTails() ; 
        auto it = relNtoN.find(relations[1]);
        if (it!=relNtoN.end()){
            NodeToNodes& tailToConst = it->second;
            // tail appears
            auto itTailToConst = tailToConst.find(tail);
            if (itTailToConst!=tailToConst.end()){
                Nodes cands = itTailToConst->second;
                //constant appears in head with tail
                if (cands.find(constants[1])!=cands.end() && !filterSet.contains(constants[0])){
                    headResults.insertRule(constants[0], this);
                    return true;
                    }
                }
            }
    }else{
        // h(X,c) <-- b1(d,X) or h(X,c) <-- b1(X,d) we want to predict all x=X that ground the body
        // tail==constants[0] is already checked
        RelNodeToNodes& relNtoN = directions[0] ? triples.getRelTailToHeads() :  triples.getRelHeadToTails();
        auto it = relNtoN.find(relations[1]);
        if (it!=relNtoN.end()){
            NodeToNodes& constToX = it->second;
            auto itconstToX = constToX.find(constants[1]);
            if (itconstToX != constToX.end()){
                for (auto ent: itconstToX->second){
                    if (!filterSet.contains(ent)){
                        headResults.insertRule(ent, this);
                    }
                }
                return true;
            }
        } 
    }
    return false;

}



bool RuleC::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults,  ManySet filterSet){
    // can only predict my constant in the grounded direction
    if (!leftC && tail!=constants[0]){
        return false;
    }

    if (directions.size()==1){
        return predictL1HeadQuery(tail, triples, headResults, filterSet);
    }

    // the internal representation for the rules is
    // h(c,Y) <-- b1(d,A), b2(A,Y)
    // h(X,c) <-- b1(X,A), b2(A,d)
    // the DFS always starts from the first body atom so for the second rule (leftC==false)
    // we turn everything around: b2(A,d),b1(X,A) starting with current_entity=d
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
                if (leftC && cEnt == tail && !filterSet.contains(constants[0])){
                    headResults.insertRule(constants[0], this);
                    return true;
                }else if(!leftC && !filterSet.contains(cEnt)){
                    headResults.insertRule(cEnt, this);
                    madePred = true;
                }
            }
            return madePred;
        }
    }
    return false;
}

// same implementation as in RuleB except that rels dirs is used depending on leftC s.t.
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
    if ((it!=relNtoN.end())){
        NodeToNodes& NtoN = it->second;
        auto entIt = NtoN.find(currEntity);
        if (entIt!=NtoN.end()){
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


// ***RuleZ implementation*** 

double RuleZ::zConfWeight = 1.0;

RuleZ::RuleZ(int& relation, bool& leftC, int& constant) {
    this->relation=relation;
    this->targetRel=relation;
    this->leftC = leftC;
    this->constant = constant;
    confWeight = RuleZ::zConfWeight;
}


bool RuleZ::predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet){
    if (leftC && !filterSet.contains(constant)){
        headResults.insertRule(constant, this);
        return true;
    }
    return false;
}

bool RuleZ::predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet){
     if (!leftC && !filterSet.contains(constant)){
        tailResults.insertRule(constant, this);
        return true;
    }
    return false;
}

std::set<Triple> RuleZ::materialize(TripleStorage& triples){
    std::set<Triple> predictions;
    // predict c when h(c,X)<-- given all h(--, a) in train and vice versa
    RelNodeToNodes& relNtoN = leftC ? triples.getRelTailToHeads() : triples.getRelHeadToTails();
    //TODO optimize
    auto it = relNtoN.find(relation);
    if (it!=relNtoN.end()){
        NodeToNodes& NtoN = it->second;
        for (auto const& pair: NtoN){
            std::pair<std::set<Triple>::iterator, bool> isNew;
            Triple triple;
            // source node 
            const int& e = pair.first;
            if (leftC){
                triple = {constant, relation, e};
                isNew = predictions.insert(triple);
            }else{
                triple = {e, relation, constant};
                isNew = predictions.insert(triple);
            }
            if (trackInMaterialize && isNew.second){
                predicted += 1;
                if (triples.contains(triple[0], triple[1], triple[2])){
                    cpredicted += 1;
                }
            }
        }
    }
    return predictions;
}


// ***RuleD implementation*** 

double RuleD::dConfWeight = 1.0;

RuleD::RuleD(std::vector<int>& relations, std::vector<bool>& directions, bool& leftC, int constant) {
    this->relations=relations;
    this->targetRel=relations[0];
    this->leftC = leftC;
    this->constant = constant;
    confWeight = RuleD::dConfWeight;
}




