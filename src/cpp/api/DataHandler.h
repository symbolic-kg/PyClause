#ifndef DATAHANDLER_H
#define DATAHANDLER_H


#include "Api.h"


#include "../core/Index.h"
#include "../core/TripleStorage.h"
#include "../core/Rule.h"
#include "../core/RuleStorage.h"
#include "../core/RuleFactory.h"
#include "../core/Globals.h"
#include "../features/Application.h"
#include "../core/Index.h"
#include "../core/Util.hpp"
#include "../core/Types.h"

#include <array>
#include <vector>
#include <string>



// Handles Rules and Triples
class DataHandler{
public:
    DataHandler(std::map<std::string, std::string> options);

    //exposed python functions
    void loadData(std::string dataPath, std::string filterPath, std::string targetPath);
    void loadData(std::string dataPath, std::string filterPath);
    void loadData(std::string dataPath);
    void loadData(std::vector<std::array<int, 3>> data, std::vector<std::array<int, 3>> filter, std::vector<std::array<int, 3>> target);
    void loadData(std::vector<std::array<int, 3>> data, std::vector<std::array<int, 3>> filter);
    void loadData(std::vector<std::array<int, 3>> data);
    void loadData(std::vector<std::array<std::string, 3>> data, std::vector<std::array<std::string, 3>> filter, std::vector<std::array<std::string, 3>> target);
    void loadData(std::vector<std::array<std::string, 3>> data, std::vector<std::array<std::string, 3>> filter);
    void loadData(std::vector<std::array<std::string, 3>> data);

    void loadRules(std::string rulePath);
    void loadRules(std::vector<std::string> ruleStrings);
    
    std::unordered_map<std::string, int>& getNodeToIdx();
	std::unordered_map<std::string, int>& getRelationToIdx();
    void subsEntityStrings(std::map<std::string, std::string>& newNames);
    void subsRelationStrings(std::map<std::string, std::string>& newNames);
    std::vector<std::string> getRuleIdx();




    //TODO probably remove, too complicated
    void setOptions(std::map<std::string, std::string> options);
    void setRuleOptions(std::map<std::string, std::string> options, RuleFactory& ruleFactory);



    // internal
    TripleStorage& getData();
    TripleStorage& getFilter();
    TripleStorage& getTarget();
    RuleStorage& getRules();
    RuleFactory& getRuleFactory();
    std::shared_ptr<Index> getIndex();
    bool getLoadedData();
    bool getLoadedRules();
    void setNodeIndex(std::vector<std::string>& idxToNode);
	void setRelIndex(std::vector<std::string>& idxToRel);
    




private:
    std::shared_ptr<Index> index;
    std::unique_ptr<TripleStorage> data;
    std::unique_ptr<TripleStorage> filter;
    std::unique_ptr<TripleStorage> target;
    std::shared_ptr<RuleFactory> ruleFactory;
    std::unique_ptr<RuleStorage> rules;

    bool loadedRules = false;
    bool loadedData = false;

    bool verbose = true;



};



#endif