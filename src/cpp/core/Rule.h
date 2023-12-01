#ifndef RULE_H
#define RULE_H

#include <vector>
#include <math.h>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include "stdio.h"
#include <array>
#include <set>


#include "Types.h"
#include "TripleStorage.h"
#include "ManySet.h"
#include "QueryResults.h"
#include "Index.h"


class Rule 
{
public:
	Rule() : 
        ID(-1),
        length(0), 
        predicted(0),
        cpredicted(0),
        bodyhash(0),
        sampledPredicted(0),
        sampledCpredicted(0),
		targetRel(0),
		rulestring(""),
		trackInMaterialize(false),
		confWeight(1.0),
		numUnseen(5)
	{
    std::ostringstream ss;
    ss << static_cast<const void*>(this);
    rulestring = ss.str();
	};
	void setID(int ID);
	void print();
	//Getter
	int getID();
	double getConfidence(int nUnseen, bool exact=false);
	double getConfidence(bool exact=false);
	void setStats(int cpredicted, int predicted, bool exact=false);
	std::array<int,2> getStats(bool exact=false);
	std::string getRuleString();
	// Maybe substitute getRuleString with this function
	virtual std::string computeRuleString(Index* index);
	long long getBodyHash();
	void computeBodyHash();
	int getTargetRel();
	// se documentation in child classes
	std::vector<int>& getRelations();
    std::vector<bool>& getDirections();
	// fully materialize rule
	virtual void materialize(TripleStorage& triples, std::unordered_set<Triple>& preds);
	// make prediction for partly grounded triples r(s,?) and r(?,o)
	// we directly store the results in a query based result structure NodeToPredRults
	virtual bool predictHeadQuery(
		int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet()
		);
	virtual bool predictTailQuery(
		int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet()
	);

	// predict a triple; can track groundings if groundings is not null
	// see documentation of child classes
	virtual bool predictTriple(
		int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings
	);


	void setTrackInMaterialize(bool val);
	void setConfWeight(double weight);
	void setRuleString(std::string str);
	void setNumUnseen(int val);

	// only used for Uxxd Uxxc rules when particularly parsed from Anyburl rule files
	virtual void setPredictHead(bool ind);
	virtual void setPredictTail(bool ind);

	bool predictHead;
	bool predictTail;
	
protected:
	int ID;
	//body length
	int length;
	// num predictions in train
	int predicted;
	// correctly predicted
	int cpredicted;
	long long bodyhash;
	// possibly sampled confidence metrics
	int sampledPredicted;
	int sampledCpredicted;
	// weight confidence with this value; is 1 per default but used for ruleZ
	double confWeight;

	// laplace smoothing for confidence prediction
	int numUnseen;

	// track exact num(correct)predicted when running materialize
	bool trackInMaterialize;

	std::string rulestring;
	// internal rule representation
	// see child classes
	std::vector<int> relations;
	std::vector<bool> directions;


	// recursive DFS step with optional grounding tracking and a target closing entity (for scoring triples)
    // used for scoring triples, e.g., DFS search but with a target end point (targetEntity)
    // can also be used to track all the groundings (list of triples) 
	// used by B, C and D rules
	void searchCurrTargetGroundings(
		int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
		int targetEntity, std::vector<int>& rels, std::vector<bool>& dirs, std::vector<Triple>& currentGroundings,
		RuleGroundings* groundings, bool& reachedTarget, bool invertGrounding=false
	);


	
	int targetRel;

	
private:

};

inline bool compareRule::operator()(Rule* lhs, Rule* rhs) const {
    double lconf = lhs->getConfidence(false);
    double rconf = rhs->getConfidence(false);
    if (lconf != rconf) {
        return lconf > rconf;
    }else{
		return lhs->getRuleString() > rhs->getRuleString();
	}
}


// ***RuleB: closed connected cyclical rules***

class RuleB: public Rule 
{
public:
	RuleB(std::vector<int>& relations, std::vector<bool>& directions);
	//TODO return semantics
	void materialize(TripleStorage& triples, std::unordered_set<Triple>& preds);
	// we directly store the results in a query based result structure NodeToPredRults
	//head query: tail given predict heads; vice versa for head query
	bool predictHeadQuery(int tail, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());
	bool predictTailQuery(int head, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());


	int branchingFactor = -1;

	// predict triple and optionally tracks grounding
	// uses searchCurrTargetGroundings()
	bool predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings);

	std::string computeRuleString(Index* index);





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
	//when relations [r1,r2,r3] then _relations [r1, r3, r2] (r1 is head relation)
	// when directions [1,0] then _directions [0,1]
	std::vector<int> _relations;
	std::vector<bool> _directions;
	
	// vanilla recursive DFS step
	void searchCurrGroundings(
		int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
		Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs 
	);
};


// RuleC rules with 2 constants (U_c rule)
class RuleC: public Rule
{
public:
	RuleC(std::vector<int>& relations, std::vector<bool>& directions, bool& leftC, std::array<int, 2>& constants);
	//TODO return semantics
	void materialize(TripleStorage& triples, std::unordered_set<Triple>& preds);
	bool predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());
	bool predictL1HeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());

	bool predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());
	bool predictL1TailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());
	
	// predict triple and optionally tracks grounding
	// uses searchCurrTargetGroundings()
	bool predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings);

	std::string computeRuleString(Index* index);
	



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

	bool predictL1Triple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings);

};


// zero rule, no body and one constant in head
// r1(X,c) <--{}  or r1(c,Y)<--{}
// note that the inference semantic of this rule is s.t. it it can only predict its constant
// like a conditional probability 
// e.g. r1(X,c) <-- can only predict c given r1(a,?) 
class RuleZ: public Rule
{
public:
	RuleZ(int& relation, bool& leftC, int& constant);
	//TODO return semantics
	void materialize(TripleStorage& triples, std::unordered_set<Triple>& preds);
	bool predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());
	bool predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());
	double getConfidence(int nUnseen, bool exact);
	std::string computeRuleString(Index* index);
	// predict triple (grounding tracking returns empty vector)
	bool predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings);
private:
	bool leftC;
	int constant;
	int relation;
};


//U_d rule, internal representation: (grounded variable does not appear in body, the other head variable appears it the expected atom)
// h(X,d) <-- b1(X,A), b2(A,B), b3(B,C)
//  leftC=false, relations=[h, b1, b2, b3], directions=[1,1,1]
// h(d,Y) <-- b1(A,B), b2(B,C), b3(C,Y)
// leftC=true, relations=[h, b1, b2, b3], directions=[1,1,1]
// h(d,Y) <-- b1(A,B), b2(C,D), b3(Y,C)
// leftC=true, relations=[h, b1, b2, b3], directions=[1,0,0]
class RuleD: public Rule
{
public:
	RuleD(std::vector<int>& relations, std::vector<bool>& directions, bool& leftC, int constant);
	void materialize(TripleStorage& triples, std::unordered_set<Triple>& preds);
	bool predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());
	bool predictL1HeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());
	bool predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());
	bool predictL1TailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());

	int branchingFactor;


	// predict triple and optionally tracks grounding
	// uses searchCurrTargetGroundings()
	bool predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings);

	std::string computeRuleString(Index* index);

private:
	bool leftC;
	int constant;
	std::vector<int> _relations;
	std::vector<bool> _directions;

	void searchCurrGroundings(
		int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples,
		Nodes& closingEntities, std::vector<int>& rels, std::vector<bool>& dirs
	);
};


class RuleXXd: public Rule
{
public:
	RuleXXd(std::vector<int>& relations, std::vector<bool>& directions);
	void setPredictHead(bool ind);
	void setPredictTail(bool ind);
	bool predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());
	bool predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());
	void materialize(TripleStorage& triples, std::unordered_set<Triple>& preds);
	std::string computeRuleString(Index* index);
	bool predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings);
private:

};


class RuleXXc: public Rule
{
public:
	RuleXXc(std::vector<int>& relations, std::vector<bool>& directions, int& constant);
	void setPredictHead(bool ind);
	void setPredictTail(bool ind);
	bool predictHeadQuery(int tail, TripleStorage& triples, QueryResults& headResults, ManySet filterSet=ManySet());
	bool predictTailQuery(int head, TripleStorage& triples, QueryResults& tailResults, ManySet filterSet=ManySet());
	void materialize(TripleStorage& triples, std::unordered_set<Triple>& preds);
	std::string computeRuleString(Index* index);
	bool predictTriple(int head, int tail, TripleStorage& triples, QueryResults& qResults, RuleGroundings* groundings);
private:
	int constant;

};



#endif // RULE_H


