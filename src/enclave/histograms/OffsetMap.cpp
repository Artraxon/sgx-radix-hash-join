/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "OffsetMap.h"
#include <core/Configuration.h>

#include <stdlib.h>

#include <Enclave_t.h>
namespace hpcjoin {
namespace histograms {

OffsetMap::OffsetMap(uint32_t numberOfProcesses, LocalHistogram* localHistogram, GlobalHistogram* globalHistogram, AssignmentMap* assignment) {

	this->numberOfProcesses = numberOfProcesses;
	this->localHistogram = localHistogram;
	this->globalHistogram = globalHistogram;
	this->assignment = assignment;

	this->baseOffsets = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
	this->relativeWriteOffsets = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
	this->absoluteWriteOffsets = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
    //Last entry is for full sum
	this->localOffsets = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT + 1, sizeof(uint64_t));

}

OffsetMap::~OffsetMap() {

	free(this->baseOffsets);
	free(this->relativeWriteOffsets);
	free(this->absoluteWriteOffsets);
	free(this->localOffsets);

}

void OffsetMap::computeOffsets() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    ocall_startHistogramOffsetComputation();
	//enclave::performance::Measurements::startHistogramOffsetComputation();
#endif

	computeBaseOffsets();
	computeRelativePrivateOffsets();
	computeAbsolutePrivateOffsets();
	computeLocalOffsets();

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	ocall_stopHistogramOffsetComputation();
	//enclave::performance::Measurements::stopHistogramOffsetComputation();
#endif

}

void OffsetMap::computeBaseOffsets() {

	uint64_t *currentOffsets = (uint64_t *) calloc(this->numberOfProcesses, sizeof(uint64_t));
	uint32_t *partitionAssignment = this->assignment->getPartitionAssignment();
	uint64_t *histogram = this->globalHistogram->getGlobalHistogram();

	for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
		uint32_t assignedNode = partitionAssignment[i];
		this->baseOffsets[i] = currentOffsets[assignedNode];
		currentOffsets[assignedNode] += histogram[i];
	}

	free(currentOffsets);

}

void OffsetMap::computeRelativePrivateOffsets() {

    ocall_MPI_scan_sum(this->localHistogram->getLocalHistogram(), this->relativeWriteOffsets, hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT);

	uint64_t *histogram = this->localHistogram->getLocalHistogram();
	for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
		this->relativeWriteOffsets[i] -= histogram[i];
	}

}

void OffsetMap::computeAbsolutePrivateOffsets() {

	for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
		this->absoluteWriteOffsets[i] = this->baseOffsets[i] + this->relativeWriteOffsets[i];
	}

}

void OffsetMap::computeLocalOffsets() {

    //First entry is omitted since the offset has to be zero
    for(uint32_t i = 1; i <= hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i){
        this->localOffsets[i] = this->localOffsets[i - 1] + this->localHistogram->getLocalHistogram()[i - 1];
    }
}

uint64_t* OffsetMap::getBaseOffsets() {

	return baseOffsets;

}

uint64_t* OffsetMap::getRelativeWriteOffsets() {

	return relativeWriteOffsets;

}

uint64_t* OffsetMap::getAbsoluteWriteOffsets() {

	return absoluteWriteOffsets;

}
uint64_t* OffsetMap::getLocalOffsets(){

    return localOffsets;
}

uint64_t * OffsetMap::getLocalHistogram(){
    return this->localHistogram->getLocalHistogram();
}


} /* namespace histograms */
} /* namespace enclave */
