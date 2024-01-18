#ifndef PREDICTIONHANDLER_H
#define PREDICTIONHANDLER_H

#include "Api.h"
#include "Loader.h"
#include "../features/Application.h"

#include <array>
#include <tuple>

class PredictionHandler: public BackendHandler{
public:
    PredictionHandler(std::map<std::string, std::string> options); 
    // exposed functions
    void scoreTriples(std::vector<std::array<int, 3>> triples,  std::shared_ptr<Loader> dHandler);
    void scoreTriples(std::vector<std::array<std::string, 3>> triples,  std::shared_ptr<Loader> dHandler);
    void scoreTriples(std::string pathToTriples,  std::shared_ptr<Loader> dHandler);
    void writeExplanations(std::string& path, bool asString);
    void writeScores(std::string& path, bool asString);

    std::vector<std::array<double, 4>> getIdxScores();
    std::vector<std::array<std::string, 4>> getStrScores();


    // ugly but its just a tuple with: a list of target triples, a list of lists of predicting rules, a list of list of grounding triples to the predicting rules    
    std::tuple<std::vector<std::array<std::string,3>>, std::vector<std::vector<std::string>>,  std::vector<std::vector<std::vector<std::vector<std::array<std::string,3>>>>>> getStrExplanations();
    std::tuple<std::vector<std::array<int,3>>, std::vector<std::vector<int>>,  std::vector<std::vector<std::vector<std::vector<std::array<int,3>>>>>> getIdxExplanations();

    void setOptions(std::map<std::string, std::string> options, ApplicationHandler& scorer);
    void setOptionsFrontend(std::map<std::string, std::string> options);
private:
    ApplicationHandler scorer;
    std::shared_ptr<Index> index;
    std::shared_ptr<Loader> myDHandler;
    std::string groundingsToString(std::vector<std::vector<Triple>> groundings, bool asString);


};



#endif