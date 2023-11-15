#ifndef QAHANDLER_H
#define QAHANDLER_H

#include "Api.h"
#include "DataHandler.h"

class QAHandler: public BackendHandler{
public:
    QAHandler(std::map<std::string, std::string> options);
    //queries are (sourceEntity, relation)
    std::vector<std::vector<std::pair<std::string,double>>> answerQueries(
        std::vector<std::pair<std::string, std::string>> queries, std::shared_ptr<DataHandler> dHandler, std::string headOrTail
        );
    std::vector<std::vector<std::pair<int, double>>> answerQueries(
        std::vector<std::pair<int, int>> queries, std::shared_ptr<DataHandler> dHandler, std::string headOrTail
        );

    //reset options
    void setOptions(std::map<std::string, std::string> options);


private:
    ApplicationHandler ranker;
};


#endif