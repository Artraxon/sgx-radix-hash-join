/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "SgxGlobalHistogram.h"
#include "SgxLocalHistogram.h"
#include <algorithm>
#include <numeric>

#include <core/Configuration.h>
#include <Enclave_t.h>

namespace hpcjoin {
namespace histograms {
#define MODE (hpcjoin::core::Configuration::MODE)
SgxGlobalHistogram::SgxGlobalHistogram(hpcjoin::histograms::SgxLocalHistogram *localHistogram, hpcjoin::histograms::AssignmentMap* assignmentMap, uint32_t numberOfNodes, uint32_t nodeID) {

	this->localHistogram = localHistogram;
	this->assignmentMap = assignmentMap;
	this->numberOfNodes = numberOfNodes;
	this->nodeID = nodeID;
	this->sealedSizesCount = 0;

	this->values = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
	this->sealedSizesSize = assignmentMap->getNodePartitionHistogram()[this->nodeID] * numberOfNodes * MODE;
	this->sealedSizes = (uint64_t *) calloc(sealedSizesSize, sizeof(uint64_t));
}

SgxGlobalHistogram::~SgxGlobalHistogram() {

    free(this->sealedSizes);
	free(this->values);

}

void SgxGlobalHistogram::computeGlobalHistogram() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    //ocall_startHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::startHistogramSgxGlobalHistogramComputation();
#endif
    ocall_MPI_allreduce_sum(this->localHistogram->getLocalHistogram(), this->values, hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT);

    uint64_t partitionsResponsible = assignmentMap->getNodePartitionHistogram()[this->nodeID];
    uint64_t outputSize = partitionsResponsible * numberOfNodes * MODE;
    //Setup All-To-All
    uint64_t sendBuf[hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT * MODE];

    int sendCounts[numberOfNodes];
    if(MODE == NOCACHING){
        std::copy(assignmentMap->getNodePartitionHistogram(), assignmentMap->getNodePartitionHistogram() + numberOfNodes, sendCounts);
    } else {
        int* base = assignmentMap->getNodePartitionHistogram();
        for (int i = 0; i < numberOfNodes; ++i) {
            sendCounts[i] = base[i] * 2;
        }
    }


    int sendDisp[this->numberOfNodes];
    sendDisp[0] = 0;
    std::partial_sum(sendCounts, sendCounts + this->numberOfNodes - 1, sendDisp + 1);

    int recCounts[this->numberOfNodes];
    //We receive as many values from every node as we are responsible for a partition
    std::fill(recCounts, recCounts + this->numberOfNodes, partitionsResponsible * MODE);
    int recDisp[this->numberOfNodes];
    recDisp[0] = 0;
    std::partial_sum(recCounts, recCounts + this->numberOfNodes - 1, recDisp + 1);

    prepareSendBuffer(sendDisp, sendBuf, this->localHistogram->getLocalHistogram());
    uint64_t outBuf[outputSize];

    ocall_MPI_AllToAllv(sendBuf,
                        sendCounts, sendDisp,
                        outBuf,
                        recCounts, recDisp,
                        hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT * MODE, numberOfNodes,
                        outputSize);

    //Sorts the sealed Sizes so that they are grouped together by partition
    for (int i = 0; i < outputSize / MODE; ++i) {
        uint64_t partition = i % (partitionsResponsible * MODE);
        uint64_t node = i / (partitionsResponsible * MODE);
        if(MODE == NOCACHING){
            this->sealedSizes[partition * numberOfNodes + node] = outBuf[i];
        } else {
            this->sealedSizesCount += outBuf[i * CACHING] + 1;
            this->sealedSizes[partition * numberOfNodes * CACHING + node * CACHING] = outBuf[i * CACHING];
            this->sealedSizes[partition * numberOfNodes * CACHING + node * CACHING + 1] = outBuf[i * CACHING + 1];
        }
    }

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	//ocall_stopHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::stopHistogramSgxGlobalHistogramComputation();
#endif

}


//Sort the Histogram after the nodes that the entries are sent to, so that AllToAll Sends them to the correct node
void SgxGlobalHistogram::prepareSendBuffer(int* sendDisp, uint64_t * sendBuf, uint64_t* data){
    uint32_t counts[this->numberOfNodes];
    std::fill(counts, counts + this->numberOfNodes, 0);

    for (uint64_t p = 0; p < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT * MODE; ++p) {
        if(MODE == NOCACHING){
            uint32_t assigned = assignmentMap->getPartitionAssignment()[p];
            sendBuf[sendDisp[assigned] + counts[assigned]++] = data[p];
        } else {
            uint32_t assigned = assignmentMap->getPartitionAssignment()[p/CACHING];
            uint64_t item;
            if(p % 2 == 0){
                item = data[p/CACHING] / core::Configuration::MEMORY_BUFFER_SIZE_BYTES;
            } else {
                item = data[p/CACHING] % core::Configuration::MEMORY_BUFFER_SIZE_BYTES;
            }
            sendBuf[sendDisp[assigned] + counts[assigned]++] = item;
        }
    }
}

uint64_t SgxGlobalHistogram::getSealedSizesCount(){
    return this->sealedSizesCount;
}

uint64_t* SgxGlobalHistogram::getGlobalHistogram() {

    return this->values;

}

uint64_t* SgxGlobalHistogram::getSealedSizes(){
    return this->sealedSizes;
}




} /* namespace histograms */
} /* namespace enclave */
