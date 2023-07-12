#include "RuleStorage.h"
#include "Index.h"

#include <fstream>


RuleStorage::RuleStorage(std::shared_ptr<Index> index){
    this->index = index;
}

void RuleStorage::read(std::string path){
    int currID = 0;
	std::string line;
	std::ifstream file(path);
	if (file.is_open()) {
		while (!util::safeGetline(file, line).eof()){
			std::vector<std::string> rawrule = util::split(line, '\t');
            for (auto el: rawrule){
                std::cout<<el<<"\n";
            }
        }
    }
};