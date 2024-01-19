#ifndef QAHANDLER_H
#define QAHANDLER_H

#include "Api.h"
#include "Loader.h"
#include <fstream>

class QAHandler: public BackendHandler{
public:
    QAHandler(std::map<std::string, std::string> options);

    // exposed python functions
    //calculate query answers, queries are (sourceEntity, relation)
    void calculate_answers(std::vector<std::pair<std::string, std::string>>& queries, std::shared_ptr<Loader> dHandler, std::string headOrTail);
    void calculate_answers(std::vector<std::pair<int, int>>& queries, std::shared_ptr<Loader> dHandler, std::string headOrTail);
    void calculate_answers(std::string& queries, std::shared_ptr<Loader> dHandler, std::string headOrTail);
    std::vector<std::vector<std::pair<std::string,double>>> getStrAnswers();
    std::vector<std::vector<std::pair<int, double>>> getIdxAnswers();

    std::vector<std::vector<std::vector<int>>> getIdxRules();
    std::vector<std::vector<std::vector<std::string>>> getStrRules();

    void writeAnswers(std::string outputPath, bool strings);
    void writeRules(std::string outputPath, bool strings);
    
    void setOptions(std::map<std::string, std::string> options);
    void setOptionsFrontend(std::map<std::string, std::string> options);
    

    void setCollectRules(bool ind);

   


private:
    ApplicationHandler ranker;
    std::vector<std::pair<int, int>> queries;
    std::vector<std::vector<std::pair<int, double>>> answers;
    // for every query for every candidate a vector of rule idx's that predicted the candidate 
    std::vector<std::vector<std::vector<Rule*>>> queryRules;
    std::shared_ptr<Index> index;

    std::shared_ptr<Loader> myDHandler;

    //setable options
    bool collectRules = false;
};


#endif