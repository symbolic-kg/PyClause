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

	RelNodeToNodes& getRelHeadToTails();
	RelNodeToNodes& getRelTailToHeads();
	void read(std::string filepath);
	void add(std::string head, std::string relation, std::string tail);

	Nodes* getTforHR(int head, int relation);
	Nodes* getHforTR(int tail, int relation);
	Index* getIndex();

protected:

private:
	std::shared_ptr<Index> index;
	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;
};

#endif // TRIPLESTORAGE_H