#include "RulesHandler.h"

#include <functional>

RulesHandler::RulesHandler(std::map<std::string, std::string> options): BackendHandler(){
    setOptions(options);
}


void RulesHandler::setOptions(std::map<std::string, std::string> options){


    // register options for ranker

     struct OptionHandler {
        std::string name;
        std::function<void(std::string)> setter;
    };

    std::vector<OptionHandler> handlers = {
        {"collect_predictions", [this](std::string val) {this->setCollectPredictions(util::stringToBool(val));}},
        {"collect_statistics", [this](std::string val) {this->setCollectStats(util::stringToBool(val));}},
        {"num_threads", [this](std::string val) {this->setNumThr(std::stoi(val));}}
    };

    for (auto& handler : handlers) {
        auto opt = options.find(handler.name);
        if (opt != options.end()) {
            if (verbose){
                std::cout<< "Setting option "<<handler.name<<" to: "<<opt->second<<std::endl;
            }
            handler.setter(opt->second);
        }
    }
}


void RulesHandler::setOptionsFrontend(std::map<std::string, std::string> options){
    setOptions(options);
}


void RulesHandler::setCollectPredictions(bool ind){
    collectPredictions = ind;
}


void RulesHandler::setCollectStats(bool ind){
    collectStats = ind;
}

void RulesHandler::setNumThr(int num){
    if (num==-1){
        num_thr = omp_get_max_threads();
    }else{
        num_thr = num;
    }
}


void RulesHandler::calcRulesPredictions(std::vector<std::string>& stringRules, std::shared_ptr<Loader> dHandler){

    index = dHandler->getIndex();
    predictions.clear();
    stats.clear();
    rules.clear();

    if (!dHandler->getLoadedData()){
        throw std::runtime_error("Please load data before you calculate rule predictions/stats.");
    }

    if (!ruleFactory){
        // use your own factory to not interfere with the options of the data loader
        ruleFactory = std::make_unique<RuleFactory>(dHandler->getIndex());
        ruleFactory->setCreateRuleB(true);
        ruleFactory->setCreateRuleC(true);
        ruleFactory->setCreateRuleD(true);
        ruleFactory->setCreateRuleXXc(true);
        ruleFactory->setCreateRuleXXd(true);
        ruleFactory->setCreateRuleZ(true);
    }
    
    this->rules = stringRules;

    int numRules = rules.size();

    if (collectPredictions){
        predictions.resize(stringRules.size());
    }

    if (collectStats){
        stats.resize(stringRules.size());
    }

    int outEvery = std::max(3, numRules/10);

    if (verbose){
        std::cout<<"Starting materialization of "<<numRules<< " rules."<<std::endl;
    }

    std::vector<std::unique_ptr<Rule>> parsed_rules;
    for (int i=0; i<numRules; i++){
        std::unique_ptr<Rule> rule = ruleFactory->parseAnytimeRule(stringRules[i]);
        rule->setTrackInMaterialize(collectStats);
        if (!rule){
            throw std::runtime_error("Error in parsing rule:" + stringRules[i]);
        }
        parsed_rules.push_back(std::move(rule));
    }

    #pragma omp parallel num_threads(num_thr)
    {
        TripleStorage& data = dHandler->getData();
        std::shared_ptr<Index> index = dHandler->getIndex();

        #pragma omp for schedule(dynamic)
        for (int i=0; i<numRules; i++){
            if (i>0 && verbose && i%outEvery==0){
                std::cout<<"Materialized rule " << i << " from " << numRules << " ..."<<std::endl;
            }
            std::unordered_set<Triple> outputs;
            parsed_rules[i]->materialize(data, outputs);
            #pragma omp critical
            {   
                if (collectPredictions){
                    predictions.at(i) = outputs;
                    outputs.clear();
                }

                if (collectStats){
                    stats.at(i) = parsed_rules[i]->getStats(true);
                }    
            } 
        }
    }
}

void RulesHandler::calcRulesPredictions(std::string& rulesPath, std::shared_ptr<Loader> dHandler){

    std::vector<std::string> stringRules;
    std::string line;
	std::ifstream file(rulesPath);
	if (file.is_open())
	{
		while (!util::safeGetline(file, line).eof())
		{
            std::vector<std::string> results = util::split(line, '\t');
			if (results.size() == 4) {
                // rules with stats
                stringRules.push_back(results[3]);
			} else if (results.size() == 1) {
                stringRules.push_back(results[0]);
            } else {
				std::cout << 
                    "Unsupported Filetype, please make sure that every line either \
                    contains a properly formatted rule string or num_pred\tsupport\tconf\truleString" 
                << std::endl;
                exit(-1);
            }
		}
		file.close();
	}
	else {
		std::cout << "Unable to open file " << rulesPath << std::endl;
		exit(-1);
	}

    calcRulesPredictions(stringRules, dHandler);
}

// writes JSON LINES format (every line is a json); allows easy scrolling through data and easy loading data
void RulesHandler::writeRulesPredictions(std::string& outputPath, bool flat, bool strings){
    if (this->predictions.size() == 0 && !collectPredictions){
        throw std::runtime_error(
            "Please use option collect_predictions and calculate predictions using calculate_predictions() first."
        );
    }
    
    std::ofstream file(outputPath);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + outputPath);
    }


    if (flat) {
        // filter triple duplicates
        std::vector<std::vector<std::array<int, 3>>> std_predictions = this->getIdxPredictions();
        std::set<std::tuple<int, int, int>> setOfTriples; 
        for (auto& triple_set : std_predictions){
            for (auto& triple : triple_set){
                std::tuple<int, int, int> triple_tuple = std::make_tuple(triple[0], triple[1], triple[2]);
                setOfTriples.insert(triple_tuple); 
            }
        } 
        for (auto triple : setOfTriples){
            std::string head = strings ? index->getStringOfNodeId(std::get<0>(triple)) : std::to_string(std::get<0>(triple));
            std::string rel = strings ? index->getStringOfRelId(std::get<1>(triple)) : std::to_string(std::get<1>(triple));
            std::string tail = strings ? index->getStringOfNodeId(std::get<2>(triple)) : std::to_string(std::get<2>(triple));
            file << head << "\t" << rel << "\t" << tail << std::endl;
        }
    } else {
        
        for (int idx = 0; idx < this->predictions.size(); idx++){
            file << "{";
            file << "\"" << this->rules[idx] << "\": [" ;

            auto itr = this->predictions[idx].begin();
            for (; itr != this->predictions[idx].end(); itr++){
                Triple triple = *itr;
                std::string head = strings ? "\"" + index->getStringOfNodeId(triple[0]) + "\"" : std::to_string(triple[0]);
                std::string rel = strings ? "\"" + index->getStringOfRelId(triple[1]) + "\"" : std::to_string(triple[1]);
                std::string tail = strings ? "\"" + index->getStringOfNodeId(triple[2]) + "\"" : std::to_string(triple[2]);
                file << "[" << head << "," << rel << "," << tail << "]";
                if (std::next(itr) != this->predictions[idx].end()) {
                    file << ",";
                }
            }
            file << "]";
            file << "}";
            if (idx < this->predictions.size() -1) {
                file << "\n";
            }
        }
        
    }
    file.close();
}

void RulesHandler::writeStats(std::string& outputPath){
    if (this->stats.size() == 0 && !collectStats){
        throw std::runtime_error(
            "There are no statistics. Please use optios collect_statistics and calculate using calculate_predictions() first."
        );
    }

    std::ofstream file(outputPath);
    if (!file.is_open()) {
        throw  std::runtime_error("Failed to create file. Please check if the paths are correct: " + outputPath);
    }

    for (int idx = 0; idx < this->rules.size(); idx++){
        file << this->stats[idx][0] << "\t" << this->stats[idx][1] << "\t" << (float)this->stats[idx][1] / (float)this->stats[idx][0] << "\t" << this->rules[idx] << std::endl;
    }
    file.close();
}

std::vector<std::vector<std::array<int, 3>>> RulesHandler::getIdxPredictions(){
    if (!collectPredictions){
        throw std::runtime_error(
            "The handler has set rules_handler.collect_predictions=False. Please set the option to true before creating the handler."
            );
    }


    std::vector<std::vector<std::array<int, 3>>> out(predictions.size());
    for (int i=0; i<predictions.size(); i++){
        out[i] = std::vector<std::array<int,3>>(predictions[i].begin(), predictions[i].end());
    }
    return out;
}


std::vector<std::vector<std::array<std::string, 3>>> RulesHandler::getStrPredictions(){

     if (!collectPredictions){
        throw std::runtime_error(
            "The handler has set rules_handler.collect_predictions=False. Please set the option to true before creating the handler."
            );
    }

    std::vector<std::vector<std::array<std::string, 3>>> out(predictions.size());

    for (int i=0; i<predictions.size(); i++){
        for (auto& triple: predictions[i]){
            std::string head = index->getStringOfNodeId(triple[0]);
            std::string rel = index->getStringOfRelId(triple[1]);
            std::string tail = index->getStringOfNodeId(triple[2]);
            out[i].push_back({head, rel, tail});
        }
    }
    return out;
}

std::vector<std::array<int,2>>& RulesHandler::getStats(){
     if (!collectStats){
        throw std::runtime_error(
            "The handler has set rules_handler.collect_statistics=False. Please set the option to true before creating the handler."
            );
    }
    return stats;
}