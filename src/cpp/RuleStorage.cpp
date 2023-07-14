#include "RuleStorage.h"
#include "Index.h"
#include "Globals.h"
#include "Rule.h"

#include <fstream>


RuleStorage::RuleStorage(std::shared_ptr<Index> index){
    this->index = index;
}

void RuleStorage::readAnyTimeFormat(std::string path, bool sampled){
    int currID = 0;
	std::string line;
	std::ifstream file(path);
	if (file.is_open()) {
		while (!util::safeGetline(file, line).eof()){
			std::vector<std::string> splitline = util::split(line, '\t');
            std::string ruleString = splitline[3];
            std::unique_ptr<Rule> rule = parseAnytimeRule(ruleString);
            if (rule){
                rules.push_back(std::move(rule));
                std::cout<<"added rule\n";
            }else{
                std::cout<<"yeah was null\n";
            }
            

            // get statistics here
      
            // makes sense to set the rule string directly from here


            // for (auto el: splitline){
            //     std::cout<<el<<"\n";
            // }
        }
    }
};

// this function is adapted from https://github.com/OpenBioLink/SAFRAN/blob/master/src/RuleReader.cpp#L43
std::unique_ptr<Rule> RuleStorage::parseAnytimeRule(std::string rule) {
    std::stringstream stream(rule);

    std::string currentChars;
    std::getline(stream, currentChars, '(');
    int relID = index->getIdOfRelationstring(currentChars);

    std::string left;
    std::string right;
    std::getline(stream, left, ',');
    std::getline(stream, right, ')');

    // the head variables of the anytime format
    char firstVar = anyTimeVars[0];
    char lastVar  = anyTimeVars.back();

    std::string body;
    // move left to the implication symbol <=
    // could be any symbol as long it is enclosed within " "
    std::getline(stream, body, ' ');
    std::getline(stream, body, ' ');

    std::vector<int> relations = {relID};
    std::vector<bool> directions; 

    std::string ruleType;

    //B-rule
    if (left[0]==firstVar && right[0]==lastVar){
        ruleType="RuleB";
        std::string literal;
        int ctr = 0;
        while (!stream.eof()){
            std::getline(stream, literal, '(');
            relations.push_back(index->getIdOfRelationstring(literal));
            std::getline(stream, left, ',');
            std::getline(stream, right, ')');
            if (left[0]==anyTimeVars[ctr] && right[0] == anyTimeVars[ctr+1]){
                directions.push_back(true);
            } else if (left[0]==anyTimeVars[ctr+1] && right[0] == anyTimeVars[ctr]) {
               directions.push_back(false); 
            } else if(left[0]==anyTimeVars.back() && right[0]==anyTimeVars[ctr]){
                directions.push_back(false);
            } else if (left[0]==anyTimeVars[ctr] && right[0]==anyTimeVars.back()){
                directions.push_back(true);
            } else {
                throw std::runtime_error("Encountered a rule in parsing that I dont understand: " + rule);
            }
            std::getline(stream, literal, ',');
            std::getline(stream, literal, ' ');
            ctr+=1;
        }
        std::cout<<"Parsed Brule \n";
        
    } else if (left[0]==firstVar && right[0] != lastVar){
        // leftC=false
        //std::cout<<"ignored U_c rule\n";
    } else if (left[0]!=firstVar && right[0]==lastVar){
        //leftC = true
        //std::cout<<"Ignored U_c rule\n";

    } else{
        throw std::runtime_error("Tried to parse a rule with ParseAnytimeFormat that does not match the format\n.");
    }   
    std::unique_ptr<RuleB> ruleInst = nullptr;
    if (ruleType=="RuleB"){
        ruleInst = std::make_unique<RuleB>(relations, directions);
    } 
    return ruleInst;

}

 std::vector<std::unique_ptr<Rule>>& RuleStorage::getRules(){
    return rules;
 }