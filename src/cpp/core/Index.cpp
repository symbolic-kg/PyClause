#include "Index.h"

#include <map>


void Index::addNode(std::string& nodesstring) {
	if (nodeToId.find(nodesstring) == nodeToId.end()) {
		nodeToId[nodesstring] = maxNodeID;
		idToNode[maxNodeID] = nodesstring;
		maxNodeID++;
	}
}

void Index::addRelation(std::string& relstring) {
	if (relToId.find(relstring) == relToId.end()) {
		relToId[relstring] = maxRelID;
		idToRel[maxRelID] = relstring;
		maxRelID++;
	}
}

int Index::getNodeSize() {
	return nodeToId.size();
}

int Index::getRelSize() {
	return relToId.size();
}

int Index::getIdOfNodestring(std::string& node) {
	auto it = nodeToId.find(node);
	if (it != nodeToId.end()) {
		return it->second;
	}
	else {
			throw std::runtime_error(("Error: Node " + node + " not found in Trainingset").c_str());
	}
}

std::string Index::getStringOfNodeId(int& id) {
	auto it = idToNode.find(id);
	if (it != idToNode.end()) {
		return it->second;
	}
	else {
		std::cout << "Error: Idx " << id << " not found in idxToNode set";
		exit(-1);
	}
}

int Index::getIdOfRelationstring(std::string& relation) {
	auto it = relToId.find(relation);
	if (it != relToId.end()) {
		return it->second;
	}
	else {
		throw std::runtime_error(("Error: Relation " + relation + " not found in loaded data."));
	}
}

std::string Index::getStringOfRelId(int& id) {
	auto it = idToRel.find(id);
	if (it != idToRel.end()) {
		return it->second;
	}
	else {
		std::cout << "Error: Idx " << id << " not found in idxToRel set";
		exit(-1);
	}
}

void Index::rehash() {
	nodeToId.rehash(nodeToId.size());
	relToId.rehash(relToId.size());
	idToNode.rehash(idToNode.size());
	idToRel.rehash(idToRel.size());
}


std::unordered_map<std::string, int>& Index::getNodeToIdx(){
	return nodeToId;
}

std::unordered_map<std::string, int>& Index::getRelationToIdx(){
	return relToId;

}


// exchange the strings of entitiess with the strings found in the keys of the map
void Index::subsEntityStrings(std::map<std::string, std::string>& newStrings){
	for (const auto& pair : newStrings) {
		if (nodeToId.count(pair.first)) { 
			int tempID = nodeToId[pair.first];
			nodeToId.erase(pair.first); 
			nodeToId[pair.second] = tempID;
			idToNode[tempID] = pair.second;
    	}
	}
}
// exchange the strings of relations with the strings found in the keys of the map
void Index::subsRelationStrings(std::map<std::string, std::string>& newStrings){
	for (const auto& pair : newStrings) {
		if (relToId.count(pair.first)) { 
			int tempID = relToId[pair.first];
			relToId.erase(pair.first); 
			relToId[pair.second] = tempID;
			idToRel[tempID] = pair.second;
    	}
	}
}

void Index::setNodeIndex(std::vector<std::string>& newIdxToNode){
	idToNode.clear();
	nodeToId.clear();
	maxNodeID = 0;

	for (int i=0; i<newIdxToNode.size(); i++){
		addNode(newIdxToNode[i]);
	}
}

void Index::setRelIndex(std::vector<std::string>& newIdxToRel){
	idToRel.clear();
	relToId.clear();
	maxRelID = 0;
	for (int i=0; i<newIdxToRel.size(); i++){
		addRelation(newIdxToRel[i]);
	}
	
}