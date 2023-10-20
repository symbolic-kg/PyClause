#ifndef QAHANDLER_H
#define QAHANDLER_H

#include "Api.h"

class QAHandler: public BackendHandler{
public:
    QAHandler(std::map<std::string, std::string> options);
    void loadDatasets(std::string dataPath, std::string filterPath);

    //queries are (sourceEntity, relation)
    std::vector<std::vector<std::pair<std::string,double>>> answerQueries(std::vector<std::pair<std::string, std::string>> queries, std::string headOrTail);
    std::vector<std::vector<std::pair<int, double>>> answerQueries(std::vector<std::pair<int, int>> queries, std::string headOrTail);

    //reset options
    void setOptions(std::map<std::string, std::string> options);


private:
    //data is already defined in backend handler
    std::unique_ptr<TripleStorage> filter;
    ApplicationHandler ranker;



};


#endif