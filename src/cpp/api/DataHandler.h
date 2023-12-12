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

    template<class T>
    void loadData(T data, T filter, T target);

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

template<class T>
void DataHandler::loadData(T data, T filter, T target){
    if (typeid(data) == typeid(TripleSet) && index->getNodeSize()==0){
        throw std::runtime_error(
            "You have to set an index first with DataHandler.set_entity_index(list[string]) DataHandler._set_relation_index(list[string]) before loading idx data."
        );
    }
    if (this->verbose){
        std::cout<< "Loading triples..." << "\n"; 
    }
    if (this->loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }
    if (target.size() > 0) {
        this->target->read(target, false);
    }
    if (filter.size() > 0) {
        this->filter->read(filter, false);
    }
    this->data->read(data, true);
    if (target.size() > 0) {
        this->target->loadCSR();
    }
    if (filter.size() > 0) {
        this->filter->loadCSR();
    }
    this->loadedData = true;
};

#endif