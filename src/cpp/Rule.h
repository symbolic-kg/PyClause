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
	Rule(){};
	void setID(int ID);
	void print();
	//Getter
	int& getID();
	double getAppliedConfidence(int nUnseen);
	double getConfidence();
	std::string getRuleString();
	long long getBodyHash();
	void computeBodyHash();
	int getTargetRel();
	

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
	std::string rulestring;
	int targetRel;
private:

};

// ***RuleB***

class RuleB: public Rule {
	public:
	RuleB(std::vector<int>& relations, std::vector<bool>& directions);
	//TODO the function should not return a created vector
	// beter would be to provide a pointer/ref to the data structure where the predictions lie in
	std::vector<std::vector<int>> materialize(TripleStorage& triples);

	private:
		//array with relation ids first element is head relation
		std::vector<int> relations;
		// ordering of the implicit body variables
		// e.g., True means (X,A) False means (A,X)
		std::vector<bool> directions;

		// currAtom: idx of current atom (first body atom has second idx=1)
		// currEntity: the current entity x for which groundings substitutions for Y 
		// are searched in currRel(x,Y)
		void searchCurrGroundings(
			int currAtomIdx, int currEntity, std::set<int>& substitutions, TripleStorage& triples, Nodes& lastEntities
		);
};

#endif // RULE_H