#ifndef QAHANDLER_H
#define QAHANDLER_H

#include "Api.h"
#include "DataHandler.h"

class QAHandler: public BackendHandler{
public:
    QAHandler(std::map<std::string, std::string> options);
    //calculate query answers, queries are (sourceEntity, relation)
    void calculate_answers(std::vector<std::pair<std::string, std::string>>& queries, std::shared_ptr<DataHandler> dHandler, std::string headOrTail);
    void calculate_answers(std::vector<std::pair<int, int>>& queries, std::shared_ptr<DataHandler> dHandler, std::string headOrTail);


    std::vector<std::vector<std::pair<std::string,double>>> getStrAnswers();
    std::vector<std::vector<std::pair<int, double>>> getIdxAnswers();

    //reset options
    void setOptions(std::map<std::string, std::string> options);


private:
    ApplicationHandler ranker;
    std::vector<std::vector<std::pair<int, double>>> answers;
    std::shared_ptr<Index> index;
};


#endif