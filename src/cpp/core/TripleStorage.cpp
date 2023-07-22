//adapted inspired from https://github.com/OpenBioLink/SAFRAN/blob/master/include/TraintripleR

#include <fstream>
#include <memory>
#include "TripleStorage.h"

TripleStorage::TripleStorage(std::shared_ptr<Index> index) {
	this->index = index;		
	index->rehash();
	for (int i = 0; i < index->getRelSize(); i++) {
		relHeadToTails[i].rehash(relHeadToTails[i].size());
		relTailToHeads[i].rehash(relTailToHeads[i].size());
	}
	relHeadToTails.rehash(relHeadToTails.size());
	relTailToHeads.rehash(relTailToHeads.size());

	//csr = new CSR<int, int>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
}

//CSR<int, int>* TripleStorage::getCSR() {
//	return csr;
//}

std::unordered_map<int, std::unordered_set<int>>& TripleStorage::getRelCounter() {
	return relCounter;
}

RelNodeToNodes& TripleStorage::getRelHeadToTails() {
	return relHeadToTails;
}

RelNodeToNodes& TripleStorage::getRelTailToHeads() {
	return relTailToHeads;
}

// read a file with tab separated triples and create data
void TripleStorage::read(std::string filepath) {
	std::string line;
	std::ifstream file(filepath);
	if (file.is_open())
	{
		while (!util::safeGetline(file, line).eof())
		{
			std::istringstream iss(line);
			std::vector<std::string> results = util::split(line, '\t');
			if (results.size() != 3) {
				std::cout << "Unsupported Filetype, please make sure you have the following triple format {subject}{TAB}{predicate}{TAB}{object}" << std::endl;
				//exit(-1);
			}
			add(results[0], results[1], results[2]);
		}
		file.close();
	}
	else {
		std::cout << "Unable to open file " << filepath << std::endl;
		exit(-1);
	}
}

void TripleStorage::add(std::string head, std::string relation, std::string tail) {
	//Get ids
	index->addNode(head);
	index->addNode(tail);
	index->addRelation(relation);

	int headNodeId = index->getIdOfNodestring(head);
	int relId = index->getIdOfRelationstring(relation);
	int tailNodeId = index->getIdOfNodestring(tail);

	relHeadToTails[relId][headNodeId].insert(tailNodeId);
	relTailToHeads[relId][tailNodeId].insert(headNodeId);
	
	relCounter[headNodeId].insert(tailNodeId);
	relCounter[tailNodeId].insert(headNodeId);
}