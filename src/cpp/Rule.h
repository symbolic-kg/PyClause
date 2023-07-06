#ifndef RULE_H
#define RULE_H

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
private:

};


class RuleB: public Rule {
	public:
	RuleB(std::vector<int>& relations, std::vector<bool>& directions);
	private:
		//array with relation ids first element is head relation
		std::vector<int> relations;
		// ordering of the implicit body variables
		// e.g., True means (X,A) False means (A,X)
		std::vector<bool> directions;
};

#endif // RULE_H