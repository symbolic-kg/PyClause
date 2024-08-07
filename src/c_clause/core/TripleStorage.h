//adapted inspired from https://github.com/OpenBioLink/SAFRAN/blob/master/include/TraintripleReader.h

#ifndef TRIPLESTORAGE_H
#define TRIPLESTORAGE_H

#include <memory>
#include <map>
#include "Index.h"
#include "Types.h"
#include "Util.hpp"
#include "RelationalCSR.hpp"
#include <string>
#include <array>
#include <vector>

class TripleStorage
{
public:
	TripleStorage(std::shared_ptr<Index> index);
	std::unique_ptr<RelationalCSR> rcsr;

	RelNodeToNodes& getRelHeadToTails();
	RelNodeToNodes& getRelTailToHeads();
	void read(std::string filepath, bool loadCSR=true);
	void read(std::vector<std::array<int, 3>> triples, bool loadCSR=true);
	void read(std::vector<std::array<std::string, 3>> triples, bool loadCSR=true);
	void loadCSR();
	void add(std::string head, std::string relation, std::string tail);
	void add(int head, int relation, int tail);
	void addIdx(int head, int relation, int tail);
	// return true if the triple exists in the data, otherwise false
	bool contains(int head, int relation, int tail);

	Nodes* getTforHR(int head, int relation);
	Nodes* getHforTR(int tail, int relation);
	void getTforHR(int head, int relation, int*& begin, int& length);
	void getHforTR(int tail, int relation, int*& begin, int& length);
	Index* getIndex();
	
	RelationalCSR* getCSR();
	void calcEntityFreq();
	int getFreq(int entity);
	int getSize();
	

protected:

private:
	// num triples
	int size=0;
	std::shared_ptr<Index> index;
	RelNodeToNodes relHeadToTails;
	RelNodeToNodes relTailToHeads;
	std::unordered_map<int, int> entityFrequencies;
};

#endif // TRIPLESTORAGE_H