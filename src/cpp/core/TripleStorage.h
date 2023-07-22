//adapted inspired from https://github.com/OpenBioLink/SAFRAN/blob/master/include/TraintripleReader.h

#ifndef TRIPLESTORAGE_H
#define TRIPLESTORAGE_H

#include <memory>
#include "Index.h"
#include "Types.h"
#include "Util.hpp"

class TripleStorage
{
public:
	TripleStorage(std::shared_ptr<Index> index);

	//CSR<int, int>* getCSR();
	std::unordered_map<int, std::unordered_set<int>>& getRelCounter();
	RelNodeToNodes& getRelHeadToTails();
	RelNodeToNodes& getRelTailToHeads();
	void read(std::string filepath);
	void add(std::string head, std::string relation, std::string tail);

protected:

private:
	std::shared_ptr<Index> index;
	//CSR<int, int>* csr;
	std::unordered_map<int, std::unordered_set<int>> relCounter;
	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;
};

#endif // TRIPLESTORAGE_H