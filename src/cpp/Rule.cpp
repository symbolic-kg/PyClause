#include "Rule.h"

// ***Base Rule implementation***

void Rule::setID(int ID){
    this->ID = ID;
}

void Rule::print(){
    throw std::runtime_error("Not implemented yet");
}

int& Rule::getID(){
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

// ***RuleB implementation*** 

RuleB::RuleB(std::vector<int>& relations, std::vector<bool>& directions) {
	this->relations = relations;
    this->directions = directions;		
}
