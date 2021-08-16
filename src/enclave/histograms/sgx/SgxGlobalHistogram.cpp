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


SgxGlobalHistogram::SgxGlobalHistogram(hpcjoin::histograms::SgxLocalHistogram *localHistogram, hpcjoin::histograms::AssignmentMap* assignmentMap, uint32_t numberOfNodes, uint32_t nodeID) {

	this->localHistogram = localHistogram;
	this->assignmentMap = assignmentMap;
	this->numberOfNodes = numberOfNodes;
	this->nodeID = nodeID;

	this->values = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
	this->sealedSizes = (uint64_t *) calloc(assignmentMap->getNodePartitionHistogram()[this->nodeID] * numberOfNodes, sizeof(uint64_t));
}

SgxGlobalHistogram::~SgxGlobalHistogram() {

    free(this->sealedSizes);
	free(this->values);

}

void SgxGlobalHistogram::computeGlobalHistogram() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    ocall_startHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::startHistogramSgxGlobalHistogramComputation();
#endif
    ocall_MPI_allreduce_sum(this->localHistogram->getLocalHistogram(), this->values, hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT);

    uint64_t partitionsResponsible = assignmentMap->getNodePartitionHistogram()[this->nodeID];
    uint64_t outputSize = partitionsResponsible * numberOfNodes;
    //Setup All-To-All
    uint64_t sendBuf[hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT];
    int* sendCounts = assignmentMap->getNodePartitionHistogram();
    int sendDisp[this->numberOfNodes];
    sendDisp[0] = 0;
    std::partial_sum(sendCounts, sendCounts + this->numberOfNodes - 1, sendDisp + 1);

    int recCounts[this->numberOfNodes];
    //We receive as many values from every node as we are responsible for a partition
    std::fill(recCounts, recCounts + this->numberOfNodes, partitionsResponsible);
    int recDisp[this->numberOfNodes];
    recDisp[0] = 0;
    std::partial_sum(recCounts, recCounts + this->numberOfNodes - 1, recDisp + 1);

    prepareSendBuffer(sendDisp, sendBuf);
    uint64_t outBuf[outputSize];

    ocall_MPI_AllToAllv(sendBuf,
                        sendCounts, sendDisp,
                        outBuf,
                        recCounts, recDisp,
                        hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, numberOfNodes,
                        outputSize);

    //Sorts the sealed Sizes so that they are grouped together by partition
    for (int i = 0; i < outputSize; ++i) {
        uint64_t partition = i % partitionsResponsible;
        uint64_t node = i / numberOfNodes;
        this->sealedSizes[partition * numberOfNodes + node] = outBuf[i];
    }

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	ocall_stopHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::stopHistogramSgxGlobalHistogramComputation();
#endif

}

uint64_t* SgxGlobalHistogram::getGlobalHistogram() {

	return this->values;

}

uint64_t* SgxGlobalHistogram::getSealedSizes(){
    return this->sealedSizes;
}
//Sort the Histogram after the nodes that the entries are sent to, so that AllToAll Sends them to the correct node
void SgxGlobalHistogram::prepareSendBuffer(int* sendDisp, uint64_t * sendBuf){
    uint32_t counts[this->numberOfNodes];
    std::fill(counts, counts + this->numberOfNodes, 0);

    for (uint64_t p = 0; p < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++p) {
        uint32_t assigned = assignmentMap->getPartitionAssignment()[p];
        sendBuf[sendDisp[assigned] + counts[assigned]++] = this->localHistogram->getLocalHistogram()[p];
    }

}

} /* namespace histograms */
} /* namespace enclave */
