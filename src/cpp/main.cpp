#include <iostream>
#include <string>
#include <memory>
#include <omp.h>
#include <chrono>
#include <numeric>

#include "core/Index.h"
#include "core/TripleStorage.h"
#include "core/Rule.h"
#include "core/RuleStorage.h"
#include "features/Application.h"
#include "core/QueryResults.h"
#include "core/RuleFactory.h"

void tests(){
    std::shared_ptr<Index> index = std::make_shared<Index>();
  
    std::string dataPath = "/home/patrick/Desktop/kge/data/wnrr/train.txt";
    TripleStorage data(index);
    data.read(dataPath);
    std::shared_ptr<RuleFactory> ruleFactory = std::make_shared<RuleFactory>(index);
    RuleStorage rules(index, ruleFactory);
    std::unique_ptr<Rule> ruleB = ruleFactory->parseAnytimeRule("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)");
    size_t size = (ruleB->materialize(data)).size();
    if (!size==83){
        throw std::runtime_error("Test 1 for B-rule failed.");
    }

    
    // Test num predictions
    //276	3	0.010869565217391304	_hypernym(X,00732746) <= _synset_domain_topic_of(X,A), _derivationally_related_form(A,10225219)
    std::unique_ptr<Rule> ruleC = ruleFactory->parseAnytimeRule("_hypernym(X,00732746) <= _synset_domain_topic_of(X,A), _derivationally_related_form(A,10225219)");
    
    // code for casting to RuleC class
    // RuleC* ruleCPtr = dynamic_cast<RuleC*>(ruleC.get());
    // ruleC.release(); // Release old pointer
    // std::unique_ptr<RuleC> ruleCC(ruleCPtr); // Create new unique_ptr
    size = (ruleC->materialize(data)).size();
    if (size!=276){
        throw std::runtime_error("Test 1 for C-rule failed.");
    }


    //test me 97	3	0.030927835051546393	_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)
    ruleC = ruleFactory->parseAnytimeRule("_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)");
    size = (ruleC->materialize(data)).size();
    if (size!=97){
        throw std::runtime_error("Test 1.1 for C-rule failed.");
    }

    //Test num predictions
    // 13	2	0.15384615384615385	_derivationally_related_form(07007945,Y) <= _derivationally_related_form(A,Y), _derivationally_related_form(07007945,A)
    ruleC = ruleFactory->parseAnytimeRule("_derivationally_related_form(07007945,Y) <= _derivationally_related_form(A,Y), _derivationally_related_form(07007945,A)");
    size = (ruleC->materialize(data)).size();
    if (size!=13){
        throw std::runtime_error("Test 2 for C-rule failed.");
    }

    // flip direction of both atoms
    ruleC = ruleFactory->parseAnytimeRule("_hypernym(X,00732746) <= _synset_domain_topic_of(A,X), _derivationally_related_form(10225219,A)");
    if (ruleC->getDirections()[0] || ruleC->getDirections()[1]){
        throw std::runtime_error("Test 3 for C-rule failed");
    }

    // flip direction of both atoms
    ruleC = ruleFactory->parseAnytimeRule("_derivationally_related_form(00748155,Y) <= _derivationally_related_form(Y,A), _derivationally_related_form(A,00748155)");
    if (ruleC->getDirections()[0] || ruleC->getDirections()[1]){
        throw std::runtime_error("Test 4 for C-rule failed");
    }

    //Test num predictions
    // 59	11	0.1864406779661017	_member_meronym(12998349,Y) <= _hypernym(Y,11590783)
    ruleC = ruleFactory->parseAnytimeRule("_member_meronym(12998349,Y) <= _hypernym(Y,11590783)");
    size = (ruleC->materialize(data)).size();
    if (size!=59){
        throw std::runtime_error("Test 5 for C-rule failed.");
    }

    //Test num predictions
    //162	2	0.012345679012345678	_hypernym(X,11669921) <= _member_meronym(11911591,X)
    ruleC = ruleFactory->parseAnytimeRule("_hypernym(X,11669921) <= _member_meronym(11911591,X)");
    size = (ruleC->materialize(data)).size();
    if (size!=162){
        throw std::runtime_error("Test 6 for C-rule failed.");
    }


    // Test RuleB predictTailQuery
    ruleB = ruleFactory->parseAnytimeRule("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)");
    QueryResults preds;
    std::string node = "08791167";
    ruleB->predictTailQuery((index->getIdOfNodestring(node)),data, preds);
    if (preds.size()!=5){
        throw std::runtime_error("Test 7 for B-rule predictTailQuery failed");
    }
    // check predictTriple: obviously the triples formed with the predicted tailCandidates must be predicted
    std::vector<int> tailPreds = preds.getCandsOrdered();
    for (int tailCand: tailPreds){
        if (!ruleB->predictTriple(index->getIdOfNodestring(node), tailCand, data, preds, nullptr)){
             throw std::runtime_error("Test 7.1 for B-rule predictTriple failed");
        }
        // any other triples must not be predicted, here tail = 0
        if (ruleB->predictTriple(index->getIdOfNodestring(node), 0, data, preds, nullptr)){
             throw std::runtime_error("Test 7.2 for B-rule predictTriple failed");
        }
    }
    node = "08921850";
    preds.clear();
    ruleB->predictHeadQuery((index->getIdOfNodestring(node)), data, preds);
    if (preds.size()!=3){
        throw std::runtime_error("Test 8 for B-rule predictHeadQuery failed");
    }
    // Test RuleC predictHead/Tail query
    // tail grounding of head query fits to tail grounding of rule, predict something
    std::string node_c = "06355894"; 
    ruleC = ruleFactory->parseAnytimeRule("_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)");
    QueryResults preds_c;
    ruleC->predictHeadQuery(index->getIdOfNodestring(node_c), data, preds_c);
    if(preds_c.size()!=97){
        throw std::runtime_error("Test 9 for C-rule predictHeadQuery failed");
    }

    // tail is different to tail constant of the rule, predict nothing
    node_c = "06898352";
    preds_c.clear(); 
    ruleC->predictHeadQuery(index->getIdOfNodestring(node_c), data, preds_c);
    if (!preds_c.empty()){
        throw std::runtime_error("Test 10 for C-rule predictHeadQuery failed");

    }

    // can only make one prediction for tail queries, the tail constant of the rule
    node_c = "01835276";
    std::string correctNode = "06355894";
    preds_c.clear();
    ruleC->predictTailQuery(index->getIdOfNodestring(node_c), data, preds_c);
    if (preds_c.size()!=1 || !preds_c.contains(index->getIdOfNodestring(correctNode))){
        throw std::runtime_error("Test 11 for C-rule predictTailQuery failed");
    }

    //test leftC C rule head/tail predictions
    ruleC = ruleFactory->parseAnytimeRule("_derivationally_related_form(07007945,Y) <= _derivationally_related_form(A,Y), _derivationally_related_form(07007945,A)");
    node_c = "07007945";
    correctNode = "00548326";
    preds_c.clear();
    ruleC->predictTailQuery(index->getIdOfNodestring(node_c), data, preds_c);
    if (preds_c.size()!=13){
        throw std::runtime_error("Test 12 for C-rule failed");
    }

    //head of tail query is different from head of rule, predict nothing
    node_c = "00548326";
    preds_c.clear();
    ruleC->predictTailQuery(index->getIdOfNodestring(node_c), data, preds_c);
    if (!preds_c.empty()){
        throw std::runtime_error("Test 13 for C-rule predictHeadQuery failed");
    }

    // predict exactly your grounded head
    node_c = "00548326";
    correctNode = "07007945";
    preds_c.clear();
    ruleC->predictHeadQuery(index->getIdOfNodestring(node_c), data, preds_c);
    if (preds_c.size()!=1 || !preds_c.contains(index->getIdOfNodestring(correctNode))){
        throw std::runtime_error("Test 14 for C-rule predictTailQuery failed");
    }


    preds_c.clear();
    ruleC = ruleFactory->parseAnytimeRule("_member_meronym(02549533,Y) <= _hypernym(Y,01429349)");
    node_c = "02640093";
    ruleC->predictHeadQuery(index->getIdOfNodestring(node_c), data, preds_c);
    if (preds_c.size()!=1){
         throw std::runtime_error("Test 15 for C-rule length 1 predictHeadQuery failed");

    }

    preds_c.clear();
    ruleC = ruleFactory->parseAnytimeRule("_hypernym(X,01189282) <= _synset_domain_topic_of(X,08441203)");
    node_c = "01068012";
    ruleC->predictTailQuery(index->getIdOfNodestring(node_c), data, preds_c);
    std::string target = "01189282";
    int targetId = index->getIdOfNodestring(target);
    if (preds_c.size()!=1){
         throw std::runtime_error("Test 16 for C-rule length 1 predictTailQuery failed");
    }

    std::unique_ptr<Rule> ruleD;
    preds.clear();
     //309	2	0.006472491909385114	_hypernym(X,05653848) <= _synset_domain_topic_of(A,X)
    ruleD = ruleFactory->parseAnytimeRule("_hypernym(X,05653848) <= _synset_domain_topic_of(A,X)");
    ruleD->setTrackInMaterialize(true);
    std::set<Triple> matPreds = ruleD->materialize(data);
    std::array<int,2> stats = ruleD->getStats(true);
    if (! (stats[0]==309 & stats[1]==2) ){
         throw std::runtime_error("Test 17 for D-rule length 1 materialize failed");
    }



    // 16106 2 _derivationally_related_form(X,01264336) <= _derivationally_related_form(A,X)
    // note that a very minor bug in AnyBURL23 leads to 16108 for stats[0]
    ruleD = ruleFactory->parseAnytimeRule("_derivationally_related_form(X,01264336) <= _derivationally_related_form(A,X)");
    ruleD->setTrackInMaterialize(true);
    matPreds = ruleD->materialize(data);
    stats = ruleD->getStats(true);
    if (!(stats[0]==16106 & stats[1]==2)){
         throw std::runtime_error("Test 18 for D-rule length 1 materialize failed");
    }
    std::string nodeStr = "01264336";
    preds.clear();
    
    ruleD->predictHeadQuery(index->getIdOfNodestring(nodeStr), data, preds);
    if (preds.size()!=16108){
        throw std::runtime_error("Test 19 for D-rule length 1 predictHeadQuery failed");

    }
    preds.clear();
    std::string headStr = "01428853";
    ruleD->predictTailQuery(index->getIdOfNodestring(headStr), data, preds);
    if ( !(preds.size()==1 && preds.contains(index->getIdOfNodestring(nodeStr)))){
        throw std::runtime_error("Test 20 for D-rule length 1 predictTailQuery failed");

    }
    preds.clear();

    //706	4	0.0056657223796034	_also_see(01716491,Y) <= _also_see(Y,A)
    ruleD = ruleFactory->parseAnytimeRule("_also_see(01716491,Y) <= _also_see(Y,A)");
    ruleD->setTrackInMaterialize(true);
    matPreds = ruleD->materialize(data);
    stats = ruleD->getStats(true);
    if (!(stats[0]==706 & stats[1]==4) ){
          throw std::runtime_error("Test 21 for D-rule length 1 materialize failed");
    }

    preds.clear();
    nodeStr = "01716491";
    ruleD->predictTailQuery(index->getIdOfNodestring(nodeStr), data, preds);
    if (preds.size()!=706){
        throw std::runtime_error("Test 22 for D-rule length 1 predictTailQuery failed.");
    }   


    preds.clear();
    nodeStr = "00452512";
    std::string headCOnst = "01716491";
    ruleD->predictHeadQuery(index->getIdOfNodestring(nodeStr), data, preds);
    if (!(preds.size()==1 && preds.contains(index->getIdOfNodestring(headCOnst)))){
        throw std::runtime_error("Test 23 for D-rule length 1 predictHeadQuery failed.");

    }

    //	_member_meronym(08176077,Y) <= _has_part(A,Y), _has_part(B,A)
    // should parse to: _member_meronym(08176077,Y) <= _has_part(A,B), _has_part(B,Y)     [A,B are flipped]
    ruleD = ruleFactory->parseAnytimeRule("_member_meronym(08176077,Y) <= _has_part(A,Y), _has_part(B,A)");
    ruleD->setTrackInMaterialize(true);
    matPreds = ruleD->materialize(data);
    size = matPreds.size();
    // 	_synset_domain_topic_of(X,00543233) <= _derivationally_related_form(X,A), _derivationally_related_form(B,A)
    // should parse to same representation
    ruleD = ruleFactory->parseAnytimeRule("_synset_domain_topic_of(X,00543233) <= _derivationally_related_form(X,A), _derivationally_related_form(B,A)");


    // *** FB15k-237 Uxx rules ***
    std::shared_ptr<Index> index237 = std::make_shared<Index>();
    std::string dataPath237 = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt";
    std::shared_ptr<RuleFactory> ruleFactory237 = std::make_shared<RuleFactory>(index237);
    TripleStorage data237(index237);
    data237.read(dataPath237);



    //683 258 0.37774524158125916 /location/hud_county_place/place(me_myself_i,Y) <= /people/person/place_of_birth(A,Y)
    std::unique_ptr<Rule> ruleXXd = ruleFactory237->parseAnytimeRule("/location/hud_county_place/place(me_myself_i,Y) <= /people/person/place_of_birth(A,Y)");
    std::set<Triple> predictions;
    ruleXXd->setTrackInMaterialize(true);
    predictions = ruleXXd->materialize(data237); 
    stats = ruleXXd -> getStats(true);  
    if (!(stats[0]==683 && stats[1]==258)){
        throw std::runtime_error("Test 24 for Uxxd rule materialize failed.");
    } 

    //67	6	 /dataworld/gardening_hint/split_to(me_myself_i,Y) <= /education/educational_institution/students_graduates./education/education/major_field_of_study(Y,/m/01lj9)
    std::unique_ptr<Rule> ruleXXc = ruleFactory237->parseAnytimeRule("/dataworld/gardening_hint/split_to(me_myself_i,Y) <= /education/educational_institution/students_graduates./education/education/major_field_of_study(Y,/m/01lj9)");
  
    ruleXXc->setTrackInMaterialize(true);
    predictions = ruleXXc->materialize(data237);
    stats = ruleXXc->getStats(true);  
    if (!(stats[0]==67 && stats[1]==6)){
        throw std::runtime_error("Test 25 for Uxxc rule materialize failed.");
    } 

    // new string representation of Uxx rules
    
    // 843	258	0.30605	/education/educational_institution/campuses(X,X) <= /location/location/contains(/m/09c7w0,X)
    ruleXXc = ruleFactory237->parseAnytimeRule("/education/educational_institution/campuses(X,X) <= /location/location/contains(/m/09c7w0,X)");
    ruleXXc->setTrackInMaterialize(true);
    predictions = ruleXXc->materialize(data237);
    stats = ruleXXc->getStats(true);  
    if (!(stats[0]==843 && stats[1]==258)){
        throw std::runtime_error("Test 26 for Uxxc rule materialize failed.");
    } 

    // 552	397	0.71920	/education/educational_institution/campuses(X,X) <= /education/educational_institution/school_type(X,A)
    ruleXXd = ruleFactory237->parseAnytimeRule("/education/educational_institution/campuses(X,X) <= /education/educational_institution/school_type(X,A)");
    ruleXXd->setTrackInMaterialize(true);
    predictions = ruleXXd->materialize(data237);
    stats = ruleXXd->getStats(true);
     if (!(stats[0]==552 && stats[1]==397)){
        throw std::runtime_error("Test 27 for Uxxd rule materialize failed.");
    } 


      std::cout<<"All rule query prediction and materialize tests passed."<<std::endl;
}


void tests_groundings(){
    std::shared_ptr<Index> index = std::make_shared<Index>();
  
    std::string dataPath = "/home/patrick/Desktop/kge/data/wnrr/train.txt";
    TripleStorage data(index);
    data.read(dataPath);
    std::shared_ptr<RuleFactory> ruleFactory = std::make_shared<RuleFactory>(index);
    RuleStorage rules(index, ruleFactory);
    std::unique_ptr<Rule> ruleB;
    
    // Test RuleB predict triple
    ruleB = ruleFactory->parseAnytimeRule("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)");
    QueryResults preds;
    std::string node = "08791167";
    ruleB->predictTailQuery((index->getIdOfNodestring(node)),data, preds);
    if (preds.size()!=5){
        throw std::runtime_error("Test for B-rule predictTailQuery failed");
    }
    // check predictTriple: obviously the triples formed with the predicted tailCandidates must be predicted
    std::vector<int> tailPreds = preds.getCandsOrdered();
    for (int tailCand: tailPreds){
        if (!ruleB->predictTriple(index->getIdOfNodestring(node), tailCand, data, preds, nullptr)){
             throw std::runtime_error("Test 1 for B-rule predictTriple failed");
        }
        // any other triples must not be predicted, here tail = 0
        if (ruleB->predictTriple(index->getIdOfNodestring(node), 0, data, preds, nullptr)){
             throw std::runtime_error("Test 2 for B-rule predictTriple failed");
        }
    }

    preds.clear();
    RuleGroundings groundings;
    ruleB->predictTriple(index->getIdOfNodestring(node), tailPreds[0], data, preds, &groundings);
    // example of how to retrieve groundings
    for (std::vector<Triple> grounding: groundings[ruleB.get()]){
        for (Triple triple: grounding){
            //std::cout<<index->getStringOfNodeId(triple[0]) + " " + index->getStringOfRelId(triple[1]) + " " + index->getStringOfNodeId(triple[2])<<std::endl;
        }
        
    }
    // 1549	26	0.016785022595222725	_hypernym(X,Y) <= _verb_group(A,X), _derivationally_related_form(B,A), _derivationally_related_form(Y,B)
    ruleB = ruleFactory->parseAnytimeRule("_hypernym(X,Y) <= _verb_group(A,X), _derivationally_related_form(B,A), _derivationally_related_form(Y,B)");

    std::set<Triple>predTriples = ruleB->materialize(data);


    QueryResults qResults;
    groundings.clear();
    for (Triple triple: predTriples){
        bool madePred = ruleB->predictTriple(triple[0], triple[2], data, qResults, &groundings);
        // must predict all triples that are predicted from materialization
        if (!madePred){
            throw std::runtime_error("Test 1 for predict triple failed.");
        }
        groundings.clear();
        madePred = ruleB->predictTriple(triple[0], 0, data, qResults, &groundings);
        // must not predict any other triple (here tail=0 works)
        if (madePred){
              throw std::runtime_error("Test 2 for predict triple failed.");
        }
        groundings.clear();

        
    }

   
    // // for debugging ruleB groundings

    // std::string head = "00784727";
    // std::string rel = "_hypernym";
    // std::string tail = "00785008";

    // std::cout<<"Triple"<<std::endl;
    // std::cout<<head + " " + rel + " " + tail<<std::endl;
    // std::cout<<"Groundings:"<<std::endl;
    // bool madePred = ruleB->predictTriple(index->getIdOfNodestring(head), index->getIdOfNodestring(tail), data, qResults, &groundings);
    // for (std::vector<Triple> grounding: groundings[ruleB.get()]){
    //         std::cout<<"Next grounding:"<<std::endl;
    //         for (Triple triple: grounding){
    //             std::cout<<index->getStringOfNodeId(triple[0]) + " " + index->getStringOfRelId(triple[1]) + " " + index->getStringOfNodeId(triple[2])<<std::endl;
    //         }
    // }

    // test ruleC leftC predict triple

    std::unique_ptr<Rule> ruleC;
    ruleC = ruleFactory->parseAnytimeRule("_derivationally_related_form(07007945,Y) <= _derivationally_related_form(A,Y), _derivationally_related_form(07007945,A)");

    predTriples.clear();
    predTriples = ruleC->materialize(data);


    qResults.clear();
    groundings.clear();
    for (Triple triple: predTriples){
        bool madePred = ruleC->predictTriple(triple[0], triple[2], data, qResults, &groundings);
        // must predict all triples that are predicted from materialization
        if (!madePred){
            throw std::runtime_error("Test 2 for predict triple RuleC failed.");
        }
        groundings.clear();
        madePred = ruleB->predictTriple(triple[0], 0, data, qResults, &groundings);
        // must not predict any other triple (here tail=0 works)
        if (madePred){
              throw std::runtime_error("Test 3 for predict triple RuleC failed.");
        }
        groundings.clear();   
    }
    // debugging groundings
    // Triple triple = *predTriples.begin();
    // std::cout<<"C rule triple: ";
    // std::cout<<index->getStringOfNodeId(triple[0]) + " " << index->getStringOfRelId(triple[1]) + " "<<  index->getStringOfNodeId(triple[2])<<std::endl;
    // madePred = ruleC->predictTriple(triple[0], triple[2], data, qResults, &groundings);
    // for (std::vector<Triple> grounding: groundings[ruleC.get()]){
    //         std::cout<<"Next grounding:"<<std::endl;
    //         for (Triple triple: grounding){
    //             std::cout<<index->getStringOfNodeId(triple[0]) + " " + index->getStringOfRelId(triple[1]) + " " + index->getStringOfNodeId(triple[2])<<std::endl;
    //         }
    // }


    // test ruleC !leftC predict triple
    ruleC = ruleFactory->parseAnytimeRule("_hypernym(X,06355894) <= _synset_domain_topic_of(X,A), _synset_domain_topic_of(06355894,A)");

    predTriples.clear();
    predTriples = ruleC->materialize(data);


    qResults.clear();
    groundings.clear();
    for (Triple triple: predTriples){
        bool madePred = ruleC->predictTriple(triple[0], triple[2], data, qResults, &groundings);
        // must predict all triples that are predicted from materialization
        if (!madePred){
            throw std::runtime_error("Test 2 for predict triple RuleC failed.");
        }
        groundings.clear();
        madePred = ruleB->predictTriple(triple[0], 0, data, qResults, &groundings);
        // must not predict any other triple (here tail=0 works)
        if (madePred){
              throw std::runtime_error("Test 3 for predict triple RuleC failed.");
        }
        groundings.clear();   
    }
    // debugging groundings
    // triple = *predTriples.begin();
    // std::cout<<"C rule triple: ";
    // std::cout<<index->getStringOfNodeId(triple[0]) + " " << index->getStringOfRelId(triple[1]) + " "<<  index->getStringOfNodeId(triple[2])<<std::endl;
    // madePred = ruleC->predictTriple(triple[0], triple[2], data, qResults, &groundings);
    // for (std::vector<Triple> grounding: groundings[ruleC.get()]){
    //         std::cout<<"Next grounding:"<<std::endl;
    //         for (Triple triple: grounding){
    //             std::cout<<index->getStringOfNodeId(triple[0]) + " " + index->getStringOfRelId(triple[1]) + " " + index->getStringOfNodeId(triple[2])<<std::endl;
    //         }
    // }


    // test ruleD !left predict Triple

    //309	2	0.006472491909385114	_hypernym(X,05653848) <= _synset_domain_topic_of(A,X)
    std::unique_ptr<Rule> ruleD;
    ruleD = ruleFactory->parseAnytimeRule("_hypernym(X,05653848) <= _synset_domain_topic_of(A,X)");

    predTriples.clear();
    predTriples = ruleD->materialize(data);


    qResults.clear();
    groundings.clear();
    // test with grounding tracking
    for (Triple triple: predTriples){
        bool madePred = ruleD->predictTriple(triple[0], triple[2], data, qResults, &groundings);
        // must predict all triples that are predicted from materialization
        if (!madePred){
            throw std::runtime_error("Test 4 for predict triple RuleD failed.");
        }
        groundings.clear();
        madePred = ruleD->predictTriple(triple[0], 0, data, qResults, &groundings);
        // must not predict any other triple (here tail=0 works)
        if (madePred){
              throw std::runtime_error("Test 5 for predict triple RuleD failed.");
        }
        groundings.clear();   
    }

    // test without grounding tracking
    qResults.clear();
    groundings.clear();
    for (Triple triple: predTriples){
        bool madePred = ruleD->predictTriple(triple[0], triple[2], data, qResults, nullptr);
        // must predict all triples that are predicted from materialization
        if (!madePred){
            throw std::runtime_error("Test 6 for predict triple RuleD failed.");
        }
        groundings.clear();
        madePred = ruleD->predictTriple(triple[0], 0, data, qResults, nullptr);
        // must not predict any other triple (here tail=0 works)
        if (madePred){
              throw std::runtime_error("Test 7 for predict triple RuleD failed.");
        }
        groundings.clear();   
    }

    // grounding debugging
    // triple = *predTriples.begin();
    // std::cout<<"D rule triple: ";
    // std::cout<<index->getStringOfNodeId(triple[0]) + " " << index->getStringOfRelId(triple[1]) + " "<<  index->getStringOfNodeId(triple[2])<<std::endl;
    // madePred = ruleD->predictTriple(triple[0], triple[2], data, qResults, &groundings);
    // for (std::vector<Triple> grounding: groundings[ruleD.get()]){
    //         std::cout<<"Next grounding:"<<std::endl;
    //         for (Triple triple: grounding){
    //             std::cout<<index->getStringOfNodeId(triple[0]) + " " + index->getStringOfRelId(triple[1]) + " " + index->getStringOfNodeId(triple[2])<<std::endl;
    //         }
    // }


    // test ruleD leftC predictTriple
   ruleD = ruleFactory->parseAnytimeRule("_also_see(01716491,Y) <= _also_see(Y,A)");

    predTriples.clear();
    predTriples = ruleD->materialize(data);


    qResults.clear();
    groundings.clear();
    // test with grounding tracking
    for (Triple triple: predTriples){
        bool madePred = ruleD->predictTriple(triple[0], triple[2], data, qResults, &groundings);
        // must predict all triples that are predicted from materialization
        if (!madePred){
            throw std::runtime_error("Test 8 for predict triple RuleD failed.");
        }
        groundings.clear();
        madePred = ruleD->predictTriple(triple[0], 0, data, qResults, &groundings);
        // must not predict any other triple (here tail=0 works)
        if (madePred){
              throw std::runtime_error("Test 9 for predict triple RuleD failed.");
        }
        groundings.clear();   
    }



    std::cout<<"All predictTriple rule tests passed."<<std::endl;
}


void testTripleScoring(){
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::shared_ptr<RuleFactory> ruleFactory = std::make_shared<RuleFactory>(index);
    ruleFactory->setCreateRuleB(true);
    ruleFactory->setCreateRuleZ(false);
    ruleFactory->setCreateRuleC(true);
    ruleFactory->setCreateRuleD(true);
    ruleFactory->setCreateRuleXXd(false);
    ruleFactory->setCreateRuleXXc(false);
    RuleZ::zConfWeight = 0.01;
    RuleD::dConfWeight = 0.1;

    std::string trainPath = "/home/patrick/Desktop/PyClause/data/wnrr/train.txt";
    std::string filterPath = "/home/patrick/Desktop/PyClause/data/wnrr/valid.txt";
    std::string targetPath = "/home/patrick/Desktop/PyClause/data/wnrr/test.txt";
   
    //test
    TripleStorage target(index);
    target.read(targetPath, false);
    //valid 
    TripleStorage filter(index);
    filter.read(filterPath, false);
    std::cout<<"data loaded. \n";

    //train
    TripleStorage train(index);
    train.read(trainPath,false);

    target.loadCSR();
    train.loadCSR();
    filter.loadCSR();


    std::string rulePath = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600";
    RuleStorage rules(index, ruleFactory);
    rules.readAnyTimeFormat(rulePath, true); 

    ApplicationHandler ranker;


    // Test correct handling of a triple that is not predicted by any of the rules 
    std::vector<Triple> triples = {{3707,7,3707}};
    ranker.setScoreCollectGroundings(true);
    ranker.calculateTripleScores(triples, train, rules);

    std::vector<std::array<double, 4>>& trScores = ranker.getTripleScores();
    std::vector<std::pair<Triple, RuleGroundings>>& trGroundings = ranker.getTripleGroundings();

    if (trGroundings[0].second.size()!=0){
        throw std::runtime_error("Test 1 for triple Scoring failed. Should not predict the triple.");
    }

     if (trScores[0][3]!=0){
        throw std::runtime_error("Test 2 for triple Scoring failed. Should predict 0 for not existing triple.");
    }
    ranker.clearAll();





    // test of the scores of predictTriples match the scores of a predicted triple from a ranking
    ranker.setTopK(100);
    ranker.setDiscAtLeast(100);
    

    ranker.setSaveCandidateRules(true);
    ranker.makeRanking(target, train, rules, filter);
    std::unordered_map<int,std::unordered_map<int, CandidateConfs>>& candsConfs = ranker.getTailQcandsConfs();

    // calculate ranking and collect some triples
    triples.clear();
    std::unordered_map<int,std::unordered_map<int, NodeToPredRules>> candRules = ranker.getTailQcandsRules();
    std::vector<Rule*> predrules = candRules[7][4008][4008];
    std::vector<double> scores;
    for(auto& outer_pair : candsConfs) {
    int rel = outer_pair.first;
        for(auto& inner_pair : outer_pair.second){
            int head = inner_pair.first;
            for(auto& candidate_conf : inner_pair.second){
                int tail = candidate_conf.first;
                triples.push_back({head, rel, tail});
                scores.push_back({candidate_conf.second});
                // just take one triple per query
                break;
            }
        }
    }

    // score the collected triples
    ranker.setScoreCollectGroundings(true);
    std::cout<<"Calculating triple scores"<<std::endl;
    ranker.calculateTripleScores(triples, train, rules);

    trScores = ranker.getTripleScores();
    trGroundings = ranker.getTripleGroundings();


    // now the scores from calculating rankings for the triples must obviously be the same than the scores 
    // of calculate triples
    for (int i=0; i<trScores.size(); i++){
        bool same = (trScores[i][3] == scores[i]);
        if (!same){
            std::cout<<trScores[i][3]<<" "<<trScores[i][3]<<" "<<trScores[i][3]<<std::endl;
            throw std::runtime_error("The score of this triple does not match its score from ranking.");
        }
    }


    std::cout<<"All apply predictTriples tests passed"<<std::endl;
}

    


void timeRanking(){

    auto start = std::chrono::high_resolution_clock::now();
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::shared_ptr<RuleFactory> ruleFactory = std::make_shared<RuleFactory>(index);
    ruleFactory->setCreateRuleB(true);
    ruleFactory->setCreateRuleZ(true);
    ruleFactory->setCreateRuleC(true);
    ruleFactory->setCreateRuleD(true);
    ruleFactory->setCreateRuleXXd(true);
    ruleFactory->setCreateRuleXXc(true);
    RuleZ::zConfWeight = 0.01;
    RuleD::dConfWeight = 0.1;
    // std::string trainPath = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt";
    // std::string filterPath = "/home/patrick/Desktop/PyClause/data/fb15k-237/valid.txt";
    // std::string targetPath = "/home/patrick/Desktop/PyClause/data/fb15k-237/test.txt";


    std::string trainPath = "/home/patrick/Desktop/PyClause/data/wnrr/train.txt";
    std::string filterPath = "/home/patrick/Desktop/PyClause/data/wnrr/valid.txt";
    std::string targetPath = "/home/patrick/Desktop/PyClause/data/wnrr/test.txt";
   
    //test
    TripleStorage target(index);
    target.read(targetPath, false);
    //valid 
    TripleStorage filter(index);
    filter.read(filterPath, false);
    std::cout<<"data loaded. \n";

    //train
    TripleStorage train(index);
    train.read(trainPath,false);

    target.loadCSR();
    train.loadCSR();
    filter.loadCSR();


    // //683 258 0.37774524158125916 /location/hud_county_place/place(me_myself_i,Y) <= /people/person/place_of_birth(A,Y)
    // std::unique_ptr<Rule> ruleXXd = ruleFactory->parseAnytimeRule("/location/hud_county_place/place(me_myself_i,Y) <= /people/person/place_of_birth(A,Y)");
    // std::string node = "/m/09c7w0";
    // QueryResults preds;
    // //ruleXXd->predictHeadQuery(index->getIdOfNodestring(node), train, preds);
    // std::set<Triple> predictions;
    // ruleXXd->setTrackInMaterialize(true);
    // predictions = ruleXXd->materialize(train);
    // //TODO: test
    


    //std::string rulePath = "/home/patrick/Desktop/PyClause/data/fb15k-237/anyburl-rules-c3-3600";
    std::string rulePath = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600";
    RuleStorage rules(index, ruleFactory);
    rules.readAnyTimeFormat(rulePath, true); 

    ApplicationHandler ranker;

    ranker.setTopK(100);
    ranker.setDiscAtLeast(100);



    ranker.makeRanking(target, train, rules, filter);

    std::string rankingFile = "/home/patrick/Desktop/PyClause/local/debug/rankingFile10.txt";

    ranker.writeRanking(target, rankingFile);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start);
    std::cout << "Overall Execution time: " << duration.count() << " seconds." << std::endl;


   



}


int main(){
    
    tests_groundings();
    tests();
    testTripleScoring();
   
    //checkRuntimes();
    timeRanking();
    
   
    exit(0);


    // data loading
    std::shared_ptr<Index> index = std::make_shared<Index>();
    std::string dataPath = "/home/patrick/Desktop/PyClause/data/wnrr/train.txt";
    TripleStorage data(index);
    data.read(dataPath);
    std::cout<<"data loaded \n";
    std::cout<<"bye \n";

    //test
    std::string testPath = "/home/patrick/Desktop/PyClause/data/wnrr/test.txt";
    TripleStorage test(index);
    test.read(testPath);

    //valid 
    std::string validPath = "/home/patrick/Desktop/PyClause/data/wnrr/valid.txt";
    TripleStorage valid(index);
    valid.read(validPath);


    // rule example
    //std::vector<int> relations = {1,6,4};
    //std::vector<bool> directions = {true,true};

    //RuleB rule(relations, directions);
    //std::cout<<rule.getTargetRel()<<"\n";

    // Rule empty;
    // std::cout<<empty.getTargetRel()<<"\n";

    // // real C-rule
    // std::string rel1Str = "_instance_hypernym";
    // std::string rel2Str = "_instance_hypernym";
    // int rel1 = index->getIdOfRelationstring(rel1Str);
    // int rel2 = index->getIdOfRelationstring(rel2Str);

    // std::string c1str = "08638442";
    // std::string c2str = "08524735";
    // int c1 = index->getIdOfNodestring(c1str);
    // int c2 = index->getIdOfNodestring(c2str);
    // std::vector<int> relations = {rel1, rel2};
    // std::vector<bool> directions = {true};
    // bool leftC = false;
    // std::array<int,2> constants = {c1, c2};

    // RuleC rule(relations, directions, leftC, constants);


    //25 predictions? :"_has_part(X,Y) <= _has_part(A,X), _member_meronym(A,B), _derivationally_related_form(B,C), _derivationally_related_form(C,D), _has_part(D,Y)"
    // parse a real B rule and materialize

    //83 30 rule correct preds from christian _has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)
    std::shared_ptr<RuleFactory> ruleFactory = std::make_shared<RuleFactory>(index);
    RuleStorage rules(index, ruleFactory);
    // std::unique_ptr<Rule> ruleB = ruleFactory->parseAnytimeRule("_has_part(X,Y) <= _has_part(X,A), _member_of_domain_region(A,B), _member_of_domain_region(Y,B)");
    
    // strAtom atom;
    // std::string input = "rel1(X,Y)";
    // ruleFactory->parseAtom(input, atom);


    // std::string node_c = "06898352"; 
    // std::unique_ptr<Rule>ruleC = ruleFactory->parseAnytimeRule("_derivationally_related_form(07007945,Y) <= _derivationally_related_form(A,Y), _derivationally_related_form(07007945,A)");
    // NodeToPredRules preds_c;
    // ruleC->predictHeadQuery(index->getIdOfNodestring(node_c), data, preds_c);

    //  // print predictions for rule
    // int counter = 0;
    // for (auto pred: ruleC->materialize(data)) {
    //     counter +=1;
    //     RelNodeToNodes& relHtoT = data.getRelHeadToTails();
    //     auto it = relHtoT.find(pred[1]);
    //     NodeToNodes& HtoT = it->second;
    //     auto _it = HtoT.find(pred[0]);
    //     if (_it != HtoT.end()){
    //         if ((_it->second).count(pred[2])>0){
    //             std::cout<<"Exists in train: ";
    //         }
    //     }
    //     std::cout << "[";
    //     for (int i = 0; i < pred.size(); ++i) {
    //         if (i==0 | i==2){
    //             std::cout << index->getStringOfNodeId(pred[i]);
    //         }else{
    //             std::cout << index->getStringOfRelId(pred[i]);
    //         }
    //         if (i != pred.size() - 1) // not the last item
    //             std::cout << ", ";
    //     }
    //     std::cout << "]\n";
    // }  
    // std::cout<<"found:"<<counter<<"\n";

    // NodeToPredRules preds;
    // std::string node = "08791167";
    // ruleB->predictTailQuery((index->getIdOfNodestring(node)), data, preds);
    // std::cout<<"hi"<<preds.size()<<std::endl;

    // node = "08921850";
    // preds.clear();
    // ruleB->predictHeadQuery((index->getIdOfNodestring(node)), data, preds);
    // std::cout<<"hi"<<preds.size()<<std::endl;


    
    //**** read and materialize rules ***
    std::string rulePath = "/home/patrick/Desktop/PyClause/data/wnrr/anyburl-rules-c5-3600";
    rules.readAnyTimeFormat(rulePath, true); 
    std::vector<std::unique_ptr<Rule>>& allRules = rules.getRules();

    // ranking example

    ApplicationHandler ranker;
    ranker.makeRanking(test, data, rules, valid);
    std::string rankFile = "/home/patrick/Desktop/PyClause/data/wnrr/firstRanking.txt";
    ranker.writeRanking(test, rankFile);
    return 0;


   
    // // // int ctr = 0;
    // // // for (auto& srule: allRules){
    // // //     srule->materialize(data);
    // // //     std::cout<<"materialized rule:"<<ctr<<"\n";
    // // //     ctr+=1;
    // // // }


    #pragma omp parallel
    {
    #pragma omp for
    for (int i = 0; i < allRules.size(); ++i){
        std::set<Triple> preds = allRules[i]->materialize(data);
        for (auto pred: preds){
            RelNodeToNodes& relHtoT = data.getRelHeadToTails();
            auto it = relHtoT.find(pred[1]);
            NodeToNodes& HtoT = it->second;
            auto _it = HtoT.find(pred[0]);
            if (_it != HtoT.end()){
                if ((_it->second).count(pred[2])>0){
                    // you can set the rule confidences like this
                    // pred exists in train
                    int a = 3;
                }
            }
        }

        std::cout<<"materialize rule:"<< i <<"\n";
    }
    }


    // std::cout<<"bye";

   
    return 0;
}
