#include "RuleStorage.h"
#include "Index.h"
#include "Globals.h"
#include "Rule.h"
#include "Types.h"

#include <fstream>
#include <string>


RuleStorage::RuleStorage(std::shared_ptr<Index> index){
    this->index = index;
}

void RuleStorage::readAnyTimeFormat(std::string path, bool sampled){
    int currID = 0;
	std::string line;
	std::ifstream file(path);
	if (file.is_open()) {
        int IDs = 0;
		while (!util::safeGetline(file, line).eof()){
			std::vector<std::string> splitline = util::split(line, '\t');
            std::string ruleString = splitline[3];
            std::unique_ptr<Rule> rule = parseAnytimeRule(ruleString);
            if (rule){
                rule->setID(currID);
                currID+=1;
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

std::unique_ptr<Rule> RuleStorage::parseAnytimeRule(std::string rule) {

    if(rule.find(equalityToken) != std::string::npos) {
    std::cout << "Found me_myself_i, skipping." << std::endl;
    return nullptr;
    }

    std::vector<std::string> headBody = util::splitString(rule, ruleSeparator);
    std::string headAtomStr = headBody[0];
    
    // no body
    if (headBody.size()==1){
        std::cout<<"that should be a zero rule, skipping yoo.";
        return nullptr;
    }
    std::vector<std::string> bodyAtomsStr = util::splitString(headBody[1], atomSeparator);
    size_t length = bodyAtomsStr.size();

    // parse head
    strAtom headAtom;
    parseAtom(headAtomStr, headAtom);
    // set head relation
    int relID = index->getIdOfRelationstring(headAtom[0]);
    std::vector<int> relations = {relID};
    std::vector<bool> directions; 
    bool leftC;
    std::array<int, 2> constants;

    // parse body
    std::vector<strAtom> bodyAtoms;
    for (int i=0; i<length; i++){
        strAtom b_i;
        parseAtom(bodyAtomsStr[i], b_i);
        bodyAtoms.push_back(b_i);
    }

    char firstVar = anyTimeVars[0];
    char lastVar  = anyTimeVars.back();

    std::string ruleType;
    // *** BRule ***
    if (headAtom[1][0]==firstVar && headAtom[2][0]==lastVar){
        ruleType = "RuleB";
        for (int i=0; i<length; i++){
            relations.push_back(index->getIdOfRelationstring(bodyAtoms[i][0]));
            char first, second;
            first = anyTimeVars[i];
            second = (i == length - 1) ? anyTimeVars.back() : anyTimeVars[i + 1];
            if (bodyAtoms[i][1][0]==first && bodyAtoms[i][2][0]==second){
                directions.push_back(true);
            } else if (bodyAtoms[i][1][0]==second && bodyAtoms[i][2][0]==first) {
                directions.push_back(false);
            } else {
                throw std::runtime_error("Encountered a Brule that I dont understand in parsing.");
            }
        }
    }else{
     // *** U_c, U_d rule ***
        if (length>2){
            std::cout<<"Cannot parse U_c, U_d rule with length larger than 2, skipping.";
            return nullptr;
        }

        bool bodyHasConst = false;
        symAtom checkHeadAtom;
        for (strAtom& atom: bodyAtoms){
            parseSymAtom(atom, checkHeadAtom);
        if (checkHeadAtom.containsConstant){
            bodyHasConst = true;
            }
        }
        if (!bodyHasConst){
            ruleType = "RuleD";
            std::cout<<"Found U_d rule, skipping.";
            return nullptr;
        }else{
            ruleType = "RuleC";
        }

        parseSymAtom(headAtom, checkHeadAtom);
        if (!checkHeadAtom.containsConstant){
            throw std::runtime_error("Expected a head constant but didnt get one in parsing.");
        }
        // assign head constant
        constants[0] = checkHeadAtom.constant;
        leftC = checkHeadAtom.leftC;

        
        symAtom checkBodyAtom;
        if (length==1){
            relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
            parseSymAtom(bodyAtoms[0], checkBodyAtom);
            constants[1] = checkBodyAtom.constant;
            if (leftC==checkBodyAtom.leftC){
                directions.push_back(true);
            }else{
                directions.push_back(false);
            }

        // we need to do this manually for leftC AnyTime Format is (length=2)
        // P530(Q142,Y) <= P530(A,Y), P495(Q368674,A)
        // whereas our representation; which  leads to rel and dir is
        // P530(Q142,Y) <= P495(Q368674,A), P530(A,Y)
        } else if (length==2 && leftC){
            relations.push_back(index->getIdOfRelationstring(bodyAtoms[1][0]));
            relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
            // start with the second atom which is our first atom
            parseSymAtom(bodyAtoms[1], checkBodyAtom);
            constants[1] = checkBodyAtom.constant;
            if (checkBodyAtom.leftC == leftC){
                directions.push_back(true);
            }else{
                directions.push_back(false);
            }
            //second var of first atom
             if (bodyAtoms[0][2][0]==lastVar){
                directions.push_back(true);
            }else{
                directions.push_back(false);
            }   

        // we need to do this manually for leftC=false AnyTime Format is (length=2)
        // P3373(X,Q13129708) <= P3373(A,X), P3373(A,Q4530046)
        // which is in line with our format
        }else if (length==2 && !leftC){
            relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
            relations.push_back(index->getIdOfRelationstring(bodyAtoms[1][0]));
            //first var of body atom (char)
            if (bodyAtoms[0][1][0]==firstVar){
                directions.push_back(true);
            }else{
                directions.push_back(false);
            }
            parseSymAtom(bodyAtoms[1], checkBodyAtom);
            constants[1] = checkBodyAtom.constant;
            if (checkBodyAtom.leftC == leftC){
                directions.push_back(true);
            }else{
                directions.push_back(false);
            }


        }
    } 

    if (ruleType=="RuleB"){
        return std::make_unique<RuleB>(relations, directions);
    } else if (ruleType=="RuleC"){
        return std::make_unique<RuleC>(relations, directions, leftC, constants);
    }
}



std::vector<std::unique_ptr<Rule>>& RuleStorage::getRules(){
    return rules;
 }

void RuleStorage::parseAtom(const std::string& input, strAtom& atom) {
     std::stringstream stream(input);
     // assign relation
    if (!std::getline(stream, atom[0], '(')) {
        throw std::runtime_error("Error in parseAtom unexpected format:" + input);
    }
    // assign head
    if (!std::getline(stream, atom[1], ',')) {
        throw std::runtime_error("Error in parseAtom unexpected format:" + input);
    }
    //assign tail
    if (!std::getline(stream, atom[2], ')')) {
        throw std::runtime_error("Error in parseAtom unexpected format:" + input);
    }
}

void RuleStorage::parseSymAtom(strAtom& inputAtom, symAtom& symAt){
    symAt.containsConstant = false;
    if (anyTimeVars.find(inputAtom[1]) == std::string::npos){
        symAt.containsConstant = true;
        symAt.constant = index->getIdOfNodestring(inputAtom[1]);
        symAt.leftC = true;
    }
    if (anyTimeVars.find(inputAtom[2]) == std::string::npos){
        if (symAt.containsConstant){
            throw std::runtime_error("Cannot have a constant in both slots of an atom.");
        }
        symAt.containsConstant = true;
        symAt.constant = index->getIdOfNodestring(inputAtom[2]);
        symAt.leftC = false;
    }
}

