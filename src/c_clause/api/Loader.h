#ifndef LOADER_H
#define LOADER_H


#include "Handler.h"


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
#include <omp.h>



// Handles Rules and Triples
class Loader{
public:
    Loader(std::map<std::string, std::string> options);

    template<class T>
    void loadData(T data, T filter, T target);

    void loadRules(std::string rulePath);
    void loadRules(std::vector<std::string> ruleStatsStrings);
    void loadRules(std::vector<std::string> ruleStrings, std::vector<std::pair<int,int>> ruleStats);

    void writeRules(std::string path);
    std::vector<std::string> getRuleLines();
    
    std::unordered_map<std::string, int>& getNodeToIdx();
	std::unordered_map<std::string, int>& getRelationToIdx();
    void subsEntityStrings(std::map<std::string, std::string>& newNames);
    void subsRelationStrings(std::map<std::string, std::string>& newNames);
    std::vector<std::string> getRuleIdx();

    void setOptions(std::map<std::string, std::string> options);
    void updateRules();

    

    // internal
    void setRuleOptions(std::map<std::string, std::string> options, RuleFactory& ruleFactory);
    TripleStorage& getData();
    TripleStorage& getFilter();
    TripleStorage& getTarget();
    RuleStorage& getRules();
    RuleFactory& getRuleFactory();
    std::shared_ptr<Index> getIndex();
    bool getLoadedData();
    bool getLoadedRules();
    void setNumThreads(int threads);

    //load a triple dataset (tab separated, 3 elements per line) conisting of tokens/strings into a std::vector<Triple> (Triple is std::array<int,3>)
    // note that all the strings need to be in the index already
    // no member is stored here, only a ptr is returned
    std::unique_ptr<std::vector<Triple>> loadTriplesToVec(std::string path); 
    
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

    int numThr=1;
};

template<class T>
void Loader::loadData(T data, T filter, T target){
    if (typeid(data) == typeid(TripleSet) && index->getNodeSize()==0){
        throw std::runtime_error(
            "You have to set an index first with Loader.set_entity_index(list[string]) Loader._set_relation_index(list[string]) before loading idx data."
        );
    }
    if (this->verbose){
        std::cout<< "Loading triples..." << "\n"; 
    }
    if (this->loadedData){
        throw std::runtime_error("Please load the data only once or use a new data handler.");
    }

    this->data->read(data, false);

    if (target.size() > 0) {
        this->target->read(target, false);
    }
    if (filter.size() > 0) {
        this->filter->read(filter, true); // all data is loaded, we can start loading CSR here
    }

    this->data->loadCSR();
    if (target.size() > 0) {
        this->target->loadCSR();
    }
    this->loadedData = true;

    if (verbose){
        std::cout<<"Loaded triples."<<std::endl;
    }
};

#endif