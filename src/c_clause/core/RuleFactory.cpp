
#include <string>

#include "RuleFactory.h"
#include "Globals.h"
#include "Types.h"




RuleFactory::RuleFactory(std::shared_ptr<Index> index){
    this->index = index;
}


std::unique_ptr<Rule> RuleFactory::parseUdRule(std::vector<std::string> headBody, int numPreds, int numTrue){
    // parse head
    strAtom headAtom;
    parseAtom(headBody[0], headAtom);

    if (!createRuleD){
        return nullptr;
    }
    if (numPreds>0 && DminPreds > numPreds){
            return nullptr;
        }
    if (numTrue>0 && DminCorrect > numTrue){
            return nullptr;
    }
    // doesnt matter which param we check
    if (numTrue>0 && ((double) numTrue/ (double) numPreds) < DminConf){
            return nullptr;
    }

    // set head relation
    int relID = index->getIdOfRelationstring(headAtom[0]);

    // data to fill
    std::vector<int> relations = {relID};
    std::vector<bool> directions; 


    symAtom checkHeadAtom;
    parseSymAtom(headAtom, checkHeadAtom);
    if (!checkHeadAtom.containsConstant){
        throw std::runtime_error("Expected constant in head of a U_d rule but did not get one.");
    }
    // assign head constant
    int headConstants = checkHeadAtom.constant;
    bool leftC = checkHeadAtom.leftC;

    std::vector<std::string> bodyAtomsStr = util::splitString(headBody[1], _cfg_prs_atomSeparator);

    size_t length = bodyAtomsStr.size();

    if (DmaxLength>0 && length>DmaxLength){
            return nullptr;
    }
    
    for (int i=0; i<length; i++){
        strAtom bodyAtom;
        parseAtom(bodyAtomsStr[i], bodyAtom);
        relations.push_back(index->getIdOfRelationstring(bodyAtom[0]));
      
        char second = _cfg_prs_anyTimeVars[i+1];
        if (bodyAtom[1][0] == second){
            directions.push_back(false);
        }else{
            directions.push_back(true);
        }
    }

    if (leftC){
        // for the internal vector representation
        std::reverse(relations.begin()+1, relations.end());
        std::reverse(directions.begin(), directions.end());
        directions.flip();
    }

    std::unique_ptr<RuleD> ruled = std::make_unique<RuleD>(relations, directions, leftC, headConstants);
    ruled->setNumUnseen(DnumUnseen);
    ruled->setConfWeight(DconfWeight);
    ruled->branchingFactor = DbranchingFactor;
    return std::move(ruled);
}


std::unique_ptr<Rule> RuleFactory::parseUcRule(std::vector<std::string> headBody, int numPreds, int numTrue){
    // parse head
    strAtom headAtom;
    parseAtom(headBody[0], headAtom);

    if (!createRuleC){
        return nullptr;
    }
    if (numPreds>0 && CminPreds > numPreds){
            return nullptr;
        }
    if (numTrue>0 && CminCorrect > numTrue){
            return nullptr;
    }
    // doesnt matter which param we check
    if (numTrue>0 && ((double) numTrue/ (double) numPreds) < CminConf){
            return nullptr;
    }

    // set head relation
    int relID = index->getIdOfRelationstring(headAtom[0]);

    // data to fill
    std::vector<int> relations = {relID};
    std::vector<bool> directions; 
    std::array<int, 2> constants;


    symAtom checkHeadAtom;
    parseSymAtom(headAtom, checkHeadAtom);
    if (!checkHeadAtom.containsConstant){
        throw std::runtime_error("Expected constant in head of a U_c rule but did not get one.");
    }
    // assign head constant
    constants[0] = checkHeadAtom.constant;
    bool leftC = checkHeadAtom.leftC;

    std::vector<std::string> bodyAtomsStr = util::splitString(headBody[1], _cfg_prs_atomSeparator);

    size_t length = bodyAtomsStr.size();

    if (CmaxLength>0 && length>CmaxLength){
            return nullptr;
    }
    
    for (int i=0; i<length; i++){
        strAtom bodyAtom;
        parseAtom(bodyAtomsStr[i], bodyAtom);
        relations.push_back(index->getIdOfRelationstring(bodyAtom[0]));
        if (i<length-1){
            char second = _cfg_prs_anyTimeVars[i+1];
            if (bodyAtom[1][0] == second){
                directions.push_back(false);
            }else{
                directions.push_back(true);
            }
        // last atom contains constant
        }else{
            symAtom lastAtom;
            parseSymAtom(bodyAtom, lastAtom);
            if (!lastAtom.containsConstant){
                throw std::runtime_error("Expected a constant in last atom of a c rule but did not get one.");
            }
            constants[1] = lastAtom.constant;
            // in the AnyBURL represention the constants always belongs to the back variable in
            // XABCD..Y  !leftC
            //or
            //YABCD..X leftC
            if (lastAtom.leftC){
                directions.push_back(false);
            }else{
                directions.push_back(true);
            }
        }
    }

    if (leftC){
        // for the internal vector representation
        std::reverse(relations.begin()+1, relations.end());
        std::reverse(directions.begin(), directions.end());
        directions.flip();
    }

    std::unique_ptr<RuleC> rulec = std::make_unique<RuleC>(relations, directions, leftC, constants);
    rulec->setNumUnseen(CnumUnseen);
    return std::move(rulec);
}


std::unique_ptr<Rule>RuleFactory::parseUXXrule(std::vector<std::string> headBody, int numPreds, int numTrue){
    if(!createRuleXXd){
        return nullptr;
    }

    // parse head
    strAtom headAtom;
    parseAtom(headBody[0], headAtom);
    // set head relation
    int relID = index->getIdOfRelationstring(headAtom[0]);
    std::vector<int> relations = {relID};
    std::vector<bool> directions; 


    // parse body
    strAtom bodyAtom;
    std::vector<std::string> bodyAtomsStr = util::splitString(headBody[1], _cfg_prs_atomSeparator);
    parseAtom(bodyAtomsStr[0], bodyAtom);
   
            
    size_t length = bodyAtomsStr.size();
    if (length>1){
        throw std::runtime_error("Cannot parse longer Uxx rule: " + headBody[0] + "<=" +  headBody[1]);
    } 
    relations.push_back(index->getIdOfRelationstring(bodyAtom[0]));

    if (bodyAtom[1][0]==_cfg_prs_anyTimeVars[0]){
        directions.push_back(true);
    }else if (bodyAtom[2][0]==_cfg_prs_anyTimeVars[0]){
        directions.push_back(false);
    }else{
        throw std::runtime_error("Cannot understand this rule: " + headBody[0] + "<=" +  headBody[1]);
    }

    // check for particular rule type  UXXc: constant is in body; UXXd: no constant
    symAtom symBodyAtom;
    parseSymAtom(bodyAtom, symBodyAtom);

    // UXXc rule
    if (symBodyAtom.containsConstant){
        if (createRuleXXc){
            std::unique_ptr<Rule> ruleXXc = std::make_unique<RuleXXc>(relations, directions, symBodyAtom.constant);
            ruleXXc->setNumUnseen(XXCnumUnseen);
            if (numPreds>0 && XXCminPreds > numPreds){
                return nullptr;
            }
            if (numTrue>0 && XXCminCorrect > numTrue){
                return nullptr;
            }
            // doesnt matter which param we check
            if (numTrue>0 && ((double) numTrue/ (double) numPreds) < XXCminConf){
                return nullptr;
            }      
            return std::move(ruleXXc);
        } else {
            return nullptr;
        }
    // Uxxd rule
    }else{
        if (createRuleXXd){
            std::unique_ptr<Rule> ruleXXd = std::make_unique<RuleXXd>(relations, directions);
            ruleXXd->setNumUnseen(XXDnumUnseen);
            if (numPreds>0 && XXDminPreds > numPreds){
                return nullptr;
            }
            if (numTrue>0 && XXDminCorrect > numTrue){
                return nullptr;
            }
            // doesnt matter which param we check
            if (numTrue>0 && ((double) numTrue/ (double) numPreds) < XXDminConf){
                return nullptr;
            }      
            return std::move(ruleXXd);
        }else{
            return nullptr;
        }   
    }

}

std::unique_ptr<Rule> RuleFactory::parseAnytimeRule(std::string rule, int numPreds, int numTrue) {
    std::string ruleType;
    std::vector<std::string> headBody = util::splitString(rule, _cfg_prs_ruleSeparator);
    std::string headAtomStr = headBody[0];
    // parse head
    strAtom headAtom;
    parseAtom(headAtomStr, headAtom);
    // set head relation
    int relID = index->getIdOfRelationstring(headAtom[0]);
    std::vector<int> relations = {relID};
    std::vector<bool> directions; 


    // Uxx d or c rule h(X,X) <- body
    if (headAtom[1][0]==_cfg_prs_anyTimeVars[0] && headAtom[2][0] == _cfg_prs_anyTimeVars[0] && headAtom[1].length()==1 && headAtom[2].length()==1){
        return parseUXXrule(headBody);
    }


    // UXX rule in old AnyTime format (the rule contains the equality token me_myself..)
    if(rule.find(_cfg_prs_equalityToken) != std::string::npos) {
        if (headAtomStr.find(_cfg_prs_equalityToken) != std::string::npos ){
            if (headBody.size()==1){
                // no body this would be a UxxZero rule, it is an artefact and we do not process it
                return nullptr;
            }
            bool predictHead=true;
            bool predictTail=true;

            // parse body
            strAtom bodyAtom;
            std::vector<std::string> bodyAtomsStr = util::splitString(headBody[1], _cfg_prs_atomSeparator);
            
            size_t length = bodyAtomsStr.size();
            if (length>1){
                throw std::runtime_error("Cannot parse longer Uxx rule: " + rule);
            } 
            if (bodyAtomsStr[0].find(_cfg_prs_equalityToken) != std::string::npos){
                //me_myself in  head and body we do not process this
                return nullptr;
            }
            parseAtom(bodyAtomsStr[0], bodyAtom);
            relations.push_back(index->getIdOfRelationstring(bodyAtom[0]));
            // special parsing from AnyBURL rules
            // direction and head tail usability is independent of type UXXd and UXXc
            if (headAtom[1] ==_cfg_prs_equalityToken && headAtom[2][0] ==_cfg_prs_anyTimeVars.back()){
                // head:  h(me_myself, Y);
                // we treat the head as h(X,X)  and track that it only predicts head queries
                predictTail = false;
                if (bodyAtom[1][0]==_cfg_prs_anyTimeVars.back()){
                    // body b(Y,A) we treat it as b(X,A) regarding the direction
                    directions.push_back(true);
                }else if (bodyAtom[2][0]==_cfg_prs_anyTimeVars.back()){
                    directions.push_back(false);
                }else{
                    throw std::runtime_error("Could not understand this rule: " + rule);
                }
            } else if (headAtom[1][0] ==_cfg_prs_anyTimeVars[0] && headAtom[2] ==_cfg_prs_equalityToken){
                // head:  "h(X, my_myself)";
                predictHead = false;
                if (bodyAtom[1][0]==_cfg_prs_anyTimeVars[0]){
                    directions.push_back(true);
                }else if (bodyAtom[2][0]==_cfg_prs_anyTimeVars[0]){
                    directions.push_back(false);
                }else{
                    throw std::runtime_error("Could not understand this rule: " + rule);
                }
            }else{
                throw std::runtime_error("Could not understand this rule: " + rule);
            }

            // check for particular rule type  UXXc: constant is in body; UXXd: no constant
            symAtom symBodyAtom;
            parseSymAtom(bodyAtom, symBodyAtom);

            // UXXc rule
            if (symBodyAtom.containsConstant){
                if (createRuleXXc){
                    std::unique_ptr<Rule> ruleXXc = std::make_unique<RuleXXc>(relations, directions, symBodyAtom.constant);
                    ruleXXc->setPredictHead(predictHead);
                    ruleXXc->setPredictTail(predictTail);
                    ruleXXc->setNumUnseen(XXCnumUnseen);
                    if (numPreds>0 && XXCminPreds > numPreds){
                        return nullptr;
                    }
                    if (numTrue>0 && XXCminCorrect > numTrue){
                        return nullptr;
                    }
                    // doesnt matter which param we check
                    if (numTrue>0 && ((double) numTrue/ (double) numPreds) < XXCminConf){
                        return nullptr;
                    }
                    return std::move(ruleXXc);
                } else {
                      return nullptr;
                }
            }else{
                // Uxxd rule
                if (createRuleXXd){
                    std::unique_ptr<Rule> ruleXXd = std::make_unique<RuleXXd>(relations, directions);
                    ruleXXd->setPredictHead(predictHead);
                    ruleXXd->setPredictTail(predictTail);
                    ruleXXd->setNumUnseen(XXDnumUnseen);

                    if (numPreds>0 && XXDminPreds > numPreds){
                        return nullptr;
                    }
                    if (numTrue>0 && XXDminCorrect > numTrue){
                        return nullptr;
                    }
                    // doesnt matter which param we check
                    if (numTrue>0 && ((double) numTrue/ (double) numPreds) < XXDminConf){
                        return nullptr;
                    }
                    return std::move(ruleXXd);
                }else{
                    return nullptr;
                }   
            }
        }else{
            //me_myself in body we do not process this
            return nullptr;
        }
    }
    // no body
    if (headBody.size()==1){
        ruleType = "RuleZ";
        symAtom sym;
        parseSymAtom(headAtom, sym);
        if (createRuleZ){
            std::unique_ptr<RuleZ> rulez = std::make_unique<RuleZ>(relID, sym.leftC, sym.constant);
            rulez->setNumUnseen(ZnumUnseen);
            rulez->setConfWeight(ZconfWeight);

            if (numPreds>0 && ZminPreds > numPreds){
                return nullptr;
            }
            if (numTrue>0 && ZminCorrect > numTrue){
                return nullptr;
            }
            // doesnt matter which param we check
            if (numTrue>0 && ((double) numTrue/ (double) numPreds) < ZminConf){
                return nullptr;
            }
            return  std::move(rulez);
        }
        else{
            return nullptr;
        }
        
    }
    std::vector<std::string> bodyAtomsStr = util::splitString(headBody[1], _cfg_prs_atomSeparator);
    size_t length = bodyAtomsStr.size();

  
    bool leftC;
    std::array<int, 2> constants;

    // parse body
    std::vector<strAtom> bodyAtoms;
    for (int i=0; i<length; i++){
        strAtom b_i;
        parseAtom(bodyAtomsStr[i], b_i);
        bodyAtoms.push_back(b_i);
    }

    char firstVar = _cfg_prs_anyTimeVars[0];
    char lastVar  = _cfg_prs_anyTimeVars.back();

    
    // *** BRule ***
    if (headAtom[1][0]==firstVar && headAtom[2][0]==lastVar && headAtom[1].length()==1 && headAtom[2].length()==1){
        ruleType = "RuleB";
        for (int i=0; i<length; i++){
            relations.push_back(index->getIdOfRelationstring(bodyAtoms[i][0]));
            char first, second;
            first = _cfg_prs_anyTimeVars[i];
            second = (i == length - 1) ? _cfg_prs_anyTimeVars.back() : _cfg_prs_anyTimeVars[i + 1];
            if (bodyAtoms[i][1][0]==first && bodyAtoms[i][2][0]==second){
                directions.push_back(true);
            } else if (bodyAtoms[i][1][0]==second && bodyAtoms[i][2][0]==first) {
                directions.push_back(false);
            } else {
                throw std::runtime_error("Encountered a Brule that I dont understand in parsing. " + rule);
            }
        }
    }else{
     // *** U_c, U_d rule ***
        bool bodyHasConst = false;
        symAtom checkBodyAtom;
        for (strAtom& atom: bodyAtoms){
            parseSymAtom(atom, checkBodyAtom);
            if (checkBodyAtom.containsConstant){
                bodyHasConst = true;
            }
        }
        // *** RuleD (U_d -rule) ***
        if (!bodyHasConst){
            ruleType = "RuleD";
            return parseUdRule(headBody, numPreds, numTrue);
        }else{
            ruleType = "RuleC";
            return parseUcRule(headBody, numPreds, numTrue);
        }
    } 
    // create rules
    if (ruleType=="RuleB" && createRuleB){

        if (BmaxLength>0 && directions.size()>BmaxLength){
            return nullptr;
        }

        std::unique_ptr<RuleB> ruleb = std::make_unique<RuleB>(relations, directions);
        ruleb->setNumUnseen(BnumUnseen);
        ruleb->branchingFactor = BbranchingFactor;

        if (numPreds>0 && BminPreds > numPreds){
            return nullptr;
        }
        if (numTrue>0 && BminCorrect > numTrue){
            return nullptr;
        }
        // doesnt matter which param we check
        if (numTrue>0 && ((double) numTrue/ (double) numPreds) < BminConf){
            return nullptr;
        }

        return std::move(ruleb); 

    } else if (ruleType=="RuleC" && createRuleC){
        throw std::runtime_error("Should not reach this in create rule C");
    }else if(ruleType=="RuleD" && createRuleD){
        throw std::runtime_error("Should not reach this in create rule D");
    } else {
        return nullptr;
    }
}


void RuleFactory::parseAtom(const std::string& input, strAtom& atom) {
     std::stringstream stream(input);
     // assign relation
    if (!std::getline(stream, atom[0], '(')) {
        throw std::runtime_error("Error when parsing string in parseAtom unexpected format:" + input);
    }
    std::string currentStr;
    std::getline(stream, currentStr);
    int last_par_pos = currentStr.find_last_of(')');
    if (last_par_pos == std::string::npos) {
        throw std::runtime_error("Error when parsing string in parseAtom: no closing parenthesis found in input:" + input);
    }
    currentStr = currentStr.substr(0, last_par_pos);

    std::vector<std::string> split = util::splitString(currentStr, ",");
    if (split.size()==2){
        atom[1] = split[0];
        atom[2] = split[1];
    // assumes that one slot is a variable; and one slot is an entity that contains even "," or "(", ")"     
    }else{
        if (split.back().length()==1){
            atom[2] = split.back();
            for (int i=0; i<split.size()-1; i ++){
                atom[1] += split[i];
                if (i<split.size()-2){
                    atom[1] += ",";
                }

            }
        } else if (split[0].size()==1){
            atom[1] = split[0];
            for (int i=1; i<split.size(); i++){
                atom[2] += split[i];
                if (i<split.size()-1){
                    atom[2] += ",";
                }
            }
        } else{
            throw std::runtime_error("Error when parsing string in parseAtom unexpected format:" + input);
        }
    }
}

void RuleFactory::parseSymAtom(strAtom& inputAtom, symAtom& symAt){
    symAt.containsConstant = false;
    symAt.constant = -1;
    symAt.leftC = false;
    if (_cfg_prs_anyTimeVars.find(inputAtom[1]) == std::string::npos){
        symAt.containsConstant = true;
        symAt.constant = index->getIdOfNodestring(inputAtom[1]);
        symAt.leftC = true;
    }
    if (_cfg_prs_anyTimeVars.find(inputAtom[2]) == std::string::npos){
        if (symAt.containsConstant){
            throw std::runtime_error("Cannot have a constant in both slots of an atom.");
        }
        symAt.containsConstant = true;
        symAt.constant = index->getIdOfNodestring(inputAtom[2]);
        symAt.leftC = false;
    }
}

void RuleFactory::setCreateRuleB(bool ind){
    createRuleB = ind;
}

void RuleFactory::setCreateRuleC(bool ind){
    createRuleC = ind;
}

void RuleFactory::setCreateRuleZ(bool ind){
    createRuleZ = ind;
}

void RuleFactory::setCreateRuleD(bool ind){
    createRuleD = ind;
}

void RuleFactory::setCreateRuleXXd(bool ind){
    createRuleXXd = ind;
}

void RuleFactory::setCreateRuleXXc(bool ind){
    createRuleXXc = ind;
}

void RuleFactory::setBbranchingFactor(int val){
    BbranchingFactor = val;
}


void RuleFactory::setDconfWeight(double val){
    DconfWeight = val;
}

void RuleFactory::setDbranchingFactor(int val){
    DbranchingFactor = val;
}


void RuleFactory::setZconfWeight(double val){
    ZconfWeight = val;
}

void RuleFactory::setCmaxLength(int val){
    CmaxLength = val;
}

void RuleFactory::setBmaxLength(int val){
    BmaxLength = val;
}

void RuleFactory::setDmaxLength(int val){
    DmaxLength = val;
}


void RuleFactory::setNumUnseen(int val, std::string type){
    if (type=="z"){
         ZnumUnseen = val;
    }else if(type=="b"){
        BnumUnseen= val;
    }else if(type=="c"){
        CnumUnseen = val;
    }else if(type=="xxd"){
        XXDnumUnseen = val;
    }else if(type=="xxc"){
        XXCnumUnseen = val;
    }else if (type=="d"){
        DnumUnseen = val;
    }else{
       throw std::runtime_error("Did not recognize rule type in setting num_unseen: " + type );
    }
}


void RuleFactory::setMinCorrect(int val, std::string type){
    if (type=="z"){
         ZminCorrect = val;
    }else if(type=="b"){
        BminCorrect= val;
    }else if(type=="c"){
        CminCorrect = val;
    }else if(type=="xxd"){
        XXDminCorrect = val;
    }else if(type=="xxc"){
        XXCminCorrect = val;
    }else if (type=="d"){
        DminCorrect = val;
    }else{
       throw std::runtime_error("Did not recognize rule type in setting min_correct: " + type );
    }
}

void RuleFactory::setMinPred(int val, std::string type){
    if (type=="z"){
         ZminPreds = val;
    }else if(type=="b"){
        BminPreds = val;
    }else if(type=="c"){
        CminPreds = val;
    }else if(type=="xxd"){
        XXDminPreds = val;
    }else if(type=="xxc"){
        XXCminPreds = val;
    }else if (type=="d"){
        DminPreds = val;
    }else{
       throw std::runtime_error("Did not recognize rule type in setting min_preds: " + type );
    }
}

void RuleFactory::setMinConf(double val, std::string type){
    if (type=="z"){
         ZminConf = val;
    }else if(type=="b"){
        BminConf = val;
    }else if(type=="c"){
        CminConf = val;
    }else if(type=="xxd"){
        XXDminConf = val;
    }else if(type=="xxc"){
        XXCminConf = val;
    }else if (type=="d"){
        DminConf = val;
    }else{
       throw std::runtime_error("Did not recognize rule type in setting min_preds: " + type );
    }
}


