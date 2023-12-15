
#include <string>

#include "RuleFactory.h"
#include "Globals.h"
#include "Types.h"




RuleFactory::RuleFactory(std::shared_ptr<Index> index){
    this->index = index;
}



std::unique_ptr<Rule>RuleFactory::parseUXXrule(std::vector<std::string> headBody, int numPreds, int numTrue){
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
    if (headAtom[1][0]==_cfg_prs_anyTimeVars[0] && headAtom[2][0] == _cfg_prs_anyTimeVars[0]){
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
    if (headAtom[1][0]==firstVar && headAtom[2][0]==lastVar){
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
            symAtom checkHeadAtom;
            parseSymAtom(headAtom, checkHeadAtom);
            if (!checkHeadAtom.containsConstant){
                throw std::runtime_error("Expected a head constant but didnt get one in parsing.");
            }
            // assign head constant
            constants[0] = checkHeadAtom.constant;
            leftC = checkHeadAtom.leftC;

            if (length==1){
                relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
                if (!leftC && firstVar==bodyAtoms[0][1][0]){
                    directions.push_back(true);
                }else if(!leftC && firstVar==bodyAtoms[0][2][0]){
                    directions.push_back(false);
                } else if (leftC && lastVar == bodyAtoms[0][2][0]){
                    directions.push_back(true);
                }else if (leftC && lastVar == bodyAtoms[0][1][0]){
                    directions.push_back(false);
                } else {
                    std::runtime_error("Could not parse D-rule: " + rule);
                }
            } else if (length==2){
                // the anyburl string rule representation deviates from our representation
                // anyburl (the input here) e.g. : h(c,Y) <-- someRel(A,Y) , otherRel(B,A)
                // our representation would be: h(c,Y) <-- otherRel(A,B), someRel(B,Y)
                // note (!!) not only the atoms are flipped also A and B are flipped!
                // so to get the first body atom we need to take the second body atom of the rule
                if (leftC){
                    relations.push_back(index->getIdOfRelationstring(bodyAtoms[1][0]));
                    relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
                // normal order
                }else{
                    relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
                    relations.push_back(index->getIdOfRelationstring(bodyAtoms[1][0]));
                }
               
                //our first body atom; second atom of anyburl; leftC no X appears
                if (leftC && bodyAtoms[1][1][0]==_cfg_prs_anyTimeVars[2]){
                    directions.push_back(true);
                }else if (leftC && bodyAtoms[1][1][0]==_cfg_prs_anyTimeVars[1]){
                    directions.push_back(false);
                // for not leftC anyburls first atom is also our first atom
                } else if (!leftC && firstVar == bodyAtoms[0][1][0]) {
                    directions.push_back(true);
                } else if (!leftC && firstVar == bodyAtoms[0][2][0]) {
                    directions.push_back(false);
                }
                else {
                    std::runtime_error("Could not parse D-rule: " + rule);
                }

                //our second body atom
                if (leftC && bodyAtoms[0][1][0]==_cfg_prs_anyTimeVars[1]){
                    directions.push_back(true);
                } else if (leftC && bodyAtoms[0][1][0]==lastVar){
                    directions.push_back(false);
                } else if (!leftC && bodyAtoms[1][1][0]==_cfg_prs_anyTimeVars[1]){
                    directions.push_back(true);
                } else if (!leftC && bodyAtoms[1][1][0]==_cfg_prs_anyTimeVars[2]){
                    directions.push_back(false);
                } else {
                     std::runtime_error("Could not parse D-rule: " + rule);
                }


            }else{
                std::runtime_error("Cannot parse longer AnyBURL U_d rules");
                
            }
        }else{
            ruleType = "RuleC";
            symAtom checkHeadAtom;
            parseSymAtom(headAtom, checkHeadAtom);
            if (!checkHeadAtom.containsConstant){
                throw std::runtime_error("Expected a head constant but didnt get one in parsing.");
            }
            // assign head constant
            constants[0] = checkHeadAtom.constant;
            leftC = checkHeadAtom.leftC;

            if (length==1){
                relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
                parseSymAtom(bodyAtoms[0], checkBodyAtom);
                constants[1] = checkBodyAtom.constant;
                if (leftC==checkBodyAtom.leftC){
                    directions.push_back(true);
                }else{
                    directions.push_back(false);
                }

            // we need to do this manually for leftC=true AnyTime Format is (length=2)
            // P530(Q142,Y) <= P530(A,Y), P495(Q368674,A)
            // whereas our representation; which  leads to rel and dir is
            // P530(Q142,Y) <= P495(Q368674,A), P530(A,Y)
            } else if (length==2 && leftC){
                relations.push_back(index->getIdOfRelationstring(bodyAtoms[1][0]));
                relations.push_back(index->getIdOfRelationstring(bodyAtoms[0][0]));
                // start with the second atom which is the first atom in our representation
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
    } 

    if (ruleType=="RuleB" && createRuleB){
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
        std::unique_ptr<RuleC> rulec = std::make_unique<RuleC>(relations, directions, leftC, constants);
        rulec->setNumUnseen(CnumUnseen);


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

        return std::move(rulec);
    }else if(ruleType=="RuleD" && createRuleD){
        std::unique_ptr<RuleD> ruled = std::make_unique<RuleD>(relations, directions, leftC, constants[0]);
        ruled->setNumUnseen(DnumUnseen);
        ruled->setConfWeight(DconfWeight);
        ruled->branchingFactor = DbranchingFactor;

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
        return std::move(ruled);
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
    // assign head
    if (!std::getline(stream, atom[1], ',')) {
        throw std::runtime_error("Error when parsing string in parseAtom unexpected format:" + input);
    }
    //assign tail
    if (!std::getline(stream, atom[2], ')')) {
        throw std::runtime_error("Error when parsing string in parseAtom unexpected format:" + input);
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


