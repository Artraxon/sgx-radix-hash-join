/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "SgxOffsetMap.h"

#include <stdlib.h>

#include <core/Configuration.h>

#include <Enclave_t.h>
namespace hpcjoin {
namespace histograms {

SgxOffsetMap::SgxOffsetMap(uint32_t numberOfProcesses, SgxLocalHistogram* localHistogram, SgxGlobalHistogram* globalHistogram, AssignmentMap* assignment) {

	this->numberOfProcesses = numberOfProcesses;
	this->localHistogram = localHistogram;
	this->globalHistogram = globalHistogram;
	this->assignment = assignment;

	this->baseOffsets = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
	this->relativeWriteOffsets = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
	this->absoluteWriteOffsets = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));

}

SgxOffsetMap::~SgxOffsetMap() {

	free(this->baseOffsets);
	free(this->relativeWriteOffsets);
	free(this->absoluteWriteOffsets);

}

void SgxOffsetMap::computeOffsets() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    ocall_startHistogramOffsetComputation();
	//enclave::performance::Measurements::startHistogramOffsetComputation();
#endif

	computeBaseOffsets();
	computeRelativePrivateOffsets();
	computeAbsolutePrivateOffsets();

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	ocall_stopHistogramOffsetComputation();
	//enclave::performance::Measurements::stopHistogramOffsetComputation();
#endif

}

void SgxOffsetMap::computeBaseOffsets() {

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

void SgxOffsetMap::computeRelativePrivateOffsets() {

    ocall_MPI_scan_sum(this->localHistogram->getLocalHistogram(), this->relativeWriteOffsets, hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT);

	uint64_t *histogram = this->localHistogram->getLocalHistogram();
	for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
		this->relativeWriteOffsets[i] -= histogram[i];
	}

}

void SgxOffsetMap::computeAbsolutePrivateOffsets() {

	for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
		this->absoluteWriteOffsets[i] = this->baseOffsets[i] + this->relativeWriteOffsets[i];
	}

}

uint64_t* SgxOffsetMap::getBaseOffsets() {

	return baseOffsets;

}

uint64_t* SgxOffsetMap::getRelativeWriteOffsets() {

	return relativeWriteOffsets;

}

uint64_t* SgxOffsetMap::getAbsoluteWriteOffsets() {

	return absoluteWriteOffsets;

}

uint64_t * SgxOffsetMap::getLocalHistogram(){
    return this->localHistogram->getLocalHistogram();
}


} /* namespace histograms */
} /* namespace enclave */
