/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "AssignmentMap.h"

#include <stdlib.h>
#include <Enclave_t.h>

#include <core/Configuration.h>

namespace hpcjoin {
namespace histograms {

AssignmentMap::AssignmentMap(uint32_t numberOfNodes, hpcjoin::histograms::GlobalHistogram *innerRelationGlobalHistogram, hpcjoin::histograms::GlobalHistogram *outerRelationGlobalHistogram) {

	this->numberOfNodes = numberOfNodes;
	this->innerRelationGlobalHistogram = innerRelationGlobalHistogram;
	this->outerRelationGlobalHistogram = outerRelationGlobalHistogram;
	this->assignment = (uint32_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint32_t));

}

AssignmentMap::~AssignmentMap() {

	free(this->assignment);

}

void AssignmentMap::computePartitionAssignment() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    ocall_startHistogramAssignmentComputation();
	//enclave::performance::Measurements::startHistogramAssignmentComputation();
#endif

	for(uint32_t p=0; p<hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++p) {
		assignment[p] = p % this->numberOfNodes;
	}

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
ocall_stopHistogramAssignmentComputation();
	//enclave::performance::Measurements::stopHistogramAssignmentComputation();
#endif

}

uint32_t* AssignmentMap::getPartitionAssignment() {

	return this->assignment;

}

} /* namespace histograms */
} /* namespace enclave */

