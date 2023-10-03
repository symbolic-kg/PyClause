//adapted inspired from https://github.com/OpenBioLink/SAFRAN/blob/master/include/TraintripleReader.h

#ifndef TRIPLESTORAGE_H
#define TRIPLESTORAGE_H

#include <memory>
#include "Index.h"
#include "Types.h"
#include "Util.hpp"
#include "RelationalCSR.hpp"

class TripleStorage
{
public:
	TripleStorage(std::shared_ptr<Index> index);

	RelNodeToNodes& getRelHeadToTails();
	RelNodeToNodes& getRelTailToHeads();
	void read(std::string filepath);
	void add(std::string head, std::string relation, std::string tail);
	// return true if the triple exists in the data, otherwise false
	bool contains(int head, int relation, int tail);

	Nodes* getTforHR(int head, int relation);
	Nodes* getHforTR(int tail, int relation);
	void getTforHR(int head, int relation, int*& begin, int& length);
	void getHforTR(int tail, int relation, int*& begin, int& length);
	Index* getIndex();
	std::unique_ptr<RelationalCSR> rcsr;
	RelationalCSR* getCSR();

protected:

private:
	std::shared_ptr<Index> index;
	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;
};

#endif // TRIPLESTORAGE_H