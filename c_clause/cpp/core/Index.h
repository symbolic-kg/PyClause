//inspired and adapted from https://github.com/OpenBioLink/SAFRAN/blob/master/include/Index.h

#ifndef INDEX_H
#define INDEX_H

#include <unordered_map>
#include <iostream>

#include <map>
#include <vector>

class Index {

public:
	void addNode(std::string& nodesstring);
	void addRelation(std::string& relstring);
	int getNodeSize();
	int getRelSize();
	int getIdOfNodestring(std::string& node);
	std::string getStringOfNodeId(int id);
	int getIdOfRelationstring(std::string& relation);
	std::string getStringOfRelId(int id);
	void rehash();
	std::unordered_map<std::string, int>& getNodeToIdx();
	std::unordered_map<std::string, int>& getRelationToIdx();
	// exchange the strings of entitiess with the strings found in the keys of the map
	void subsEntityStrings(std::map<std::string, std::string>& newNames);
	// exchange the strings of relations with the strings found in the keys of the map
	void subsRelationStrings(std::map<std::string, std::string>& newNames);

	void setNodeIndex(std::vector<std::string>& idxToNode);
	void setRelIndex(std::vector<std::string>& idxToRel);


private:
	std::unordered_map<std::string, int> nodeToId;
	std::unordered_map<std::string, int> relToId;
	std::unordered_map<int, std::string> idToNode;
	std::unordered_map<int, std::string> idToRel;

	int maxNodeID = 0;
	int maxRelID = 0;
};

#endif // INDEX_H