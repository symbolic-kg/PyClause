#ifndef PREDICTIONHANDLER_H
#define PREDICTIONHANDLER_H

#include "Api.h"
#include "DataHandler.h"
#include "../features/Application.h"

#include <array>
#include <tuple>

class PredictionHandler: public BackendHandler{
public:
    PredictionHandler(std::map<std::string, std::string> options); 
    // bound methods
    void scoreTriples(std::vector<std::array<int, 3>> triples,  std::shared_ptr<DataHandler> dHandler);
    void scoreTriples(std::vector<std::array<std::string, 3>> triples,  std::shared_ptr<DataHandler> dHandler);
    void scoreTriples(std::string pathToTriples,  std::shared_ptr<DataHandler> dHandler);

    std::vector<std::array<double, 4>> getIdxScores();
    std::vector<std::array<std::string, 4>> getStrScores();

    // ugly but its just a tuple with: a list of target triples, a list of lists of predicting rules, a list of list of grounding triples to the predicting rules    
    std::tuple<std::vector<std::array<std::string,3>>, std::vector<std::vector<std::string>>,  std::vector<std::vector<std::vector<std::vector<std::array<std::string,3>>>>>> getStrExplanations();
    std::tuple<std::vector<std::array<int,3>>, std::vector<std::vector<int>>,  std::vector<std::vector<std::vector<std::vector<std::array<int,3>>>>>> getIdxExplanations();
private:
    ApplicationHandler scorer;
    void setOptions(std::map<std::string, std::string> options, ApplicationHandler& scorer);
    std::shared_ptr<Index> index;
    std::shared_ptr<DataHandler> myDHandler;
};



#endif