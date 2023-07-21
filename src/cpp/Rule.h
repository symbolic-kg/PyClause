#ifndef RULE_H
#define RULE_H

#include "Types.h"
#include "TripleStorage.h"
#include "stdio.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <optional>

class Rule 
{
public:
	Rule() : 
        ID(-1),
        length(0), 
        predicted(0),
        cpredicted(0),
        confidence(0.0),
        applied_confidence(0.0),
        bodyhash(0),
        sampledPredicted(0),
        sampledCpredicted(0),
		sampledConf(0),
		targetRel(0),
		rulestring("")
	{};
	void setID(int ID);
	void print();
	//Getter
	int getID();
	double getAppliedConfidence(int nUnseen);
	double getConfidence();
	std::string getRuleString();
	long long getBodyHash();
	void computeBodyHash();
	int getTargetRel();
	// se documentation in child classes
	std::vector<int>& getRelations();
    std::vector<bool>& getDirections();
	// fully materialize rule
	virtual std::vector<std::vector<int>> materialize(TripleStorage& triples);
	// make prediction for partly grounded triples r(s,?) and r(?,o)
	// we directly store the results in a query based result structure NodeToPredRults
	virtual bool predictHeadQuery(int head, TripleStorage& triples, NodeToPredRules& tailResults);
	virtual bool predictTailQuery(int tail, TripleStorage& triples, NodeToPredRules& headResults);
protected:
	int ID;
	//body length
	int length;
	// num predictions in train
	int predicted;
	// correctly predicted
	int cpredicted;
	double confidence;
	double applied_confidence;
	long long bodyhash;
	// possibly sampled confidence metrics
	int sampledPredicted;
	int sampledCpredicted;
	int sampledConf;
	// see child classes
	std::vector<int> relations;
	std::vector<bool> directions;
	std::string rulestring;
	int targetRel;
private:

};

// ***RuleB: closed connected cyclical rules***

class RuleB: public Rule 
{
public:
	RuleB(std::vector<int>& relations, std::vector<bool>& directions);
	//TODO the function should not return a created vector
	// beter would be to provide a pointer/ref to the data structure where the predictions lie in
	std::vector<std::vector<int>> materialize(TripleStorage& triples);
	// we directly store the results in a query based result structure NodeToPredRults
	bool predictHeadQuery(int head, TripleStorage& triples, NodeToPredRules& tailResults);
	bool predictTailQuery(int tail, TripleStorage& triples, NodeToPredRules& headResults);

private:
	// this->relations and this->directions uniquely identifies a rule
	// e.g. relations=[r1, r2, r3]
	//       directions=[True, False]
	// is the rule r1(X,Y)<-r2(X,A),r3(Y,A)
	//array with relation ids first element is head relation
	//std::vector<int> relations;
	// ordering of the implicit body variables
	// e.g., True means (X,A) False means (A,X)
	// std::vector<bool> directions;

	// currAtom: idx of current atom  wrt this.relations (starts with first body atom has second idx=1)
	// currEntity: the current entity x for which groundings substitutions for Y 
	// are searched in currRel(x,Y)
	// substitutions: all entities that are assigned already to a variable on the current path
	// they must be assigned to another variables (object identity)
	// closingEntities: the end entities that are assigned to the closing variable
	// i.e., that are assigned to Y in h(X,Y)<-b1(X,A),b2(A,Y)

	//used for predicting tails
	std::vector<int> _relations;
	std::vector<bool> _directions;
	
	void searchCurrGroundings(
		int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
		Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs 
	);
};


// RuleC rules with 2 constants
class RuleC: public Rule
{
public:
	RuleC(std::vector<int>& relations, std::vector<bool>& directions, bool& leftC, std::array<int, 2>& constants);
	//TODO check RuleB todo
	std::vector<std::vector<int>> materialize(TripleStorage& triples);
	bool predictHeadQuery(int head, TripleStorage& triples, NodeToPredRules& tailResults);
	bool predictTailQuery(int tail, TripleStorage& triples, NodeToPredRules& headResults);
private:
	// this->relations, this->directions, this->leftC, this->constants, uniquely identify a C (U_c) rule
	// e.g. relations = [r1, r2, r3]
	//      directions = [false, true]
	//      leftC = false
	//       constants = [c,d]
	// r1(X,c)<--r2(A,X),r3(A,d)
	// note that when a var is grounded it can be grounded to different entities
	// for leftC = true the first body atom contains the constant relations,directions stays the same
	// r1(c,Y)<--r2(A,c),r3(A,Y)
	//std::vector<int> relations;
	//std::vector<bool> directions;
	// left body is grounded (X is grounded means it's a "Y-rule")
	// if false Y is grounded 
	bool leftC;
	std::array<int, 2> constants;

	// used internally for materialization
	// if leftC=false _direction is the inverse of directions
	// and _relations is first element equal to relations (head) remaining elements order flipped
	std::vector<int> _relations;
	std::vector<bool> _directions;

	void searchCurrGroundings(
		int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
		Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs
	);
};


#endif // RULE_H