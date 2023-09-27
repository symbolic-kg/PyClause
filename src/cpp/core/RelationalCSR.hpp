#include <numeric>
#include "Types.h"

struct CSR{
    int *rowPtr;
    int *colInd;
};


class RelationalCSR {

    public:
        RelationalCSR(int numRelations, int numNodes, RelNodeToNodes& relHeadToTails, RelNodeToNodes& relTailToHeads) {
            this->numNodes = numNodes;
            this->numRelations = numRelations;
            csrs = new CSR*[numRelations*2];
            for (int rel=0; rel < numRelations; rel++){
                csrs[rel] = getCsr(relHeadToTails[rel]);
                csrs[numRelations + rel] = getCsr(relTailToHeads[rel]);
            }
        }

        ~RelationalCSR(){
            for (int rel=0; rel < this->numRelations; rel++){
                delete csrs[rel]->colInd;
                delete csrs[rel]->rowPtr;
                delete csrs[rel];
            }
            delete csrs;
        }

        Nodes* getTforHR(int head, int relation){
            // copying dynamic array to std::vector would be more efficient and would not require new allocation/creation of set...
			// using std::vector is ~5times faster
            Nodes * nodes = new Nodes();
            int* colInd = this->csrs[relation]->colInd;
            int* rowPtr = this->csrs[relation]->rowPtr;
            nodes->insert(&colInd[rowPtr[head]], &colInd[rowPtr[head+1]]);
            return nodes;
        }

        Nodes* getHforTR(int tail, int relation){
            return getTforHR(tail, relation*2);
        }

        void getTforHREfficient(int head, int relation, int*& begin, int& length){
            int* colInd = this->csrs[relation]->colInd;
            int* rowPtr = this->csrs[relation]->rowPtr;
            begin = &colInd[rowPtr[head]];
            length = rowPtr[head+1] - rowPtr[head];
        }

        void getHforTRFast(int tail, int relation, int*& begin, int& length){
            getTforHREfficient(tail, relation, begin, length);
        }
    private:
        int numNodes;
        int numRelations;
        CSR ** csrs;

        CSR* getCsr(NodeToNodes& nodeToNodes){
            CSR *csr = new CSR();
            csr->rowPtr = new int[1 + this->numNodes]; // rowPtr[0] is always 0
            std::fill(csr->rowPtr, csr->rowPtr + (1 + numNodes), 0);
            for (auto& itNodeToNodes: nodeToNodes) {
                csr->rowPtr[1 + itNodeToNodes.first] += itNodeToNodes.second.size();
            }
            int nnz = std::accumulate(csr->rowPtr, csr->rowPtr + 1 + numNodes, 0);
            // csr->nnz = nnz;
            std::partial_sum(csr->rowPtr, csr->rowPtr + 1 + numNodes, csr->rowPtr);

            csr->colInd = new int[nnz];
            for (auto& itNodeToNodes: nodeToNodes) {
                std::copy(itNodeToNodes.second.begin(), itNodeToNodes.second.end(), &csr->colInd[csr->rowPtr[itNodeToNodes.first]]);
            }
            return csr;
        }
};