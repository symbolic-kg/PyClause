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


RelationalCSR* TripleStorage::getCSR(){
	return rcsr.get();
}


RelNodeToNodes& TripleStorage::getRelHeadToTails() {
	return relHeadToTails;
}

RelNodeToNodes& TripleStorage::getRelTailToHeads() {
	return relTailToHeads;
}

// read a file with tab separated triples and create data
// loadCSR is true per default, when multiple datasets are loaded it should be set to false
// and after all the data is loaded (and the index is constructed) the csr should be loaded
// with loadCSR() otherwise the CSR are bugged in cases where the set of entities of the different datasets differs
void TripleStorage::read(std::string filepath, bool loadCSR) {
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

	if (loadCSR){
		rcsr = std::make_unique<RelationalCSR>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
	}
}


void TripleStorage::read(std::vector<std::array<std::string, 3>> triples, bool loadCSR) {
	for(auto& triple : triples){
		add(triple[0], triple[1], triple[2]);
	}
	if (loadCSR){
		rcsr = std::make_unique<RelationalCSR>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
	}
}

void TripleStorage::read(std::vector<std::array<int, 3>> triples, bool loadCSR) {
	for(auto& triple : triples){
		add(triple[0], triple[1], triple[2]);
	}
	if (loadCSR){
		rcsr = std::make_unique<RelationalCSR>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
	}
}

void TripleStorage::loadCSR(){
	rcsr = std::make_unique<RelationalCSR>(index->getRelSize(), index->getNodeSize(), relHeadToTails, relTailToHeads);
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
	
}

// Needed a way to add idx triples without the error handling (cannot use addIdx; for e.g. loadData([[0,0,0]]))
// With error handling -> chicken and egg problem
void TripleStorage::add(int head, int relation, int tail) {
	relHeadToTails[relation][head].insert(tail);
	relTailToHeads[relation][tail].insert(head);
}

// can only add idx'es that already exist
void TripleStorage::addIdx(int head, int relation, int tail) {
	//error handling
	index->getStringOfNodeId(head);
	index->getStringOfNodeId(tail);
	index->getStringOfRelId(relation);

	relHeadToTails[relation][head].insert(tail);
	relTailToHeads[relation][tail].insert(head);
	
}


Nodes* TripleStorage::getTforHR(int head, int relation){
	auto it_rel = relHeadToTails.find(relation);
	if (it_rel!=relHeadToTails.end()){
		auto it_head = (it_rel->second).find(head);
		if (it_head!=(it_rel->second).end()){
			return &(it_head->second);
		}
	}
	return nullptr;
}
Nodes* TripleStorage::getHforTR(int tail, int relation){
	auto it_rel = relTailToHeads.find(relation);
	if (it_rel!=relTailToHeads.end()){
		auto it_head = (it_rel->second).find(tail);
		if (it_head!=(it_rel->second).end()){
			return &(it_head->second);
		}
	}
	return nullptr;
}


void TripleStorage::getTforHR(int head, int relation, int*& begin, int& length){
	rcsr->getTforHREfficient(head, relation, begin, length);

}
void TripleStorage::getHforTR(int tail, int relation, int*& begin, int& length){
	rcsr->getHforTREfficient(tail, relation, begin, length);

}

Index* TripleStorage::getIndex(){
	return index.get();
}

bool TripleStorage::contains(int head, int relation, int tail){
	auto it = relHeadToTails.find(relation);
	if (it!=relHeadToTails.end()){
		NodeToNodes& HtoT = it->second;
		auto _it = HtoT.find(head);
		if (_it != HtoT.end()){
        	if((_it->second).count(tail)>0){
                 return true;
        	}
	 	}
	}
	return false;
}

void TripleStorage::calcEntityFreq(){
	for (int r=0; r<index->getRelSize(); r++){
		for (int i=0; i<index->getNodeSize(); i++){
			int* beginH;
			int lengthH;
			getHforTR(i, r, beginH, lengthH);
			entityFrequencies[i] += lengthH;
			int* beginT;
			int lengthT;
			getTforHR(i, r, beginT, lengthT);
			entityFrequencies[i] += lengthT;
		}
	}
}

int TripleStorage::getFreq(int ent){
	return entityFrequencies[ent];
}

