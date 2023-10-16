#include <iostream>
#include <string>
#include <memory>
#include <omp.h>
#include <chrono>
#include <numeric>

#include "core/myClass.h"
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
    std::cout<<"All tests passed."<<std::endl;
    ruleXXc->setTrackInMaterialize(true);
    predictions = ruleXXc->materialize(data237);
    stats = ruleXXc -> getStats(true);  
    if (!(stats[0]==67 && stats[1]==6)){
        throw std::runtime_error("Test 25 for Uxxc rule materialize failed.");
    } 

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
    RuleD::dConfWeight = 0.01;
    std::string trainPath = "/home/patrick/Desktop/PyClause/data/fb15k-237/train.txt";
    std::string filterPath = "/home/patrick/Desktop/PyClause/data/fb15k-237/valid.txt";
    std::string targetPath = "/home/patrick/Desktop/PyClause/data/fb15k-237/test.txt";
   
    //test
    TripleStorage target(index);
    target.read(targetPath);
    //valid 
    TripleStorage filter(index);
    filter.read(filterPath);
    std::cout<<"data loaded. \n";

    //train
    TripleStorage train(index);
    train.read(trainPath);


    //683 258 0.37774524158125916 /location/hud_county_place/place(me_myself_i,Y) <= /people/person/place_of_birth(A,Y)
    std::unique_ptr<Rule> ruleXXd = ruleFactory->parseAnytimeRule("/location/hud_county_place/place(me_myself_i,Y) <= /people/person/place_of_birth(A,Y)");
    std::string node = "/m/09c7w0";
    QueryResults preds;
    //ruleXXd->predictHeadQuery(index->getIdOfNodestring(node), train, preds);
    std::set<Triple> predictions;
    ruleXXd->setTrackInMaterialize(true);
    predictions = ruleXXd->materialize(train);
    //TODO: test
    


    std::string rulePath = "/home/patrick/Desktop/PyClause/data/fb15k-237/anyburl-rules-c3-3600";
    RuleStorage rules(index, ruleFactory);
    std::cout<<"here";
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
    tests();
    //checkRuntimes();
    timeRanking();
    
   
    exit(0);





    myClass ob("peter"); 
    long start = 1;
    long end = 100000;
    std::cout<<ob.addRange(start,end)<<"\n";

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
