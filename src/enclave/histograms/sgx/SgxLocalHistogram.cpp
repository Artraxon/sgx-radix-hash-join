/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "SgxLocalHistogram.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <Enclave_t.h>
#include <sgx_tseal.h>

#include <core/Configuration.h>

namespace hpcjoin {
namespace histograms {


SgxLocalHistogram::SgxLocalHistogram(uint64_t * localHistogram, uint32_t numberOfNodes, uint64_t localSize) {

    this->localSize = localSize;
	this->localHistogram = localHistogram;
	this->numberOfNodes = numberOfNodes;

	this->values = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));

}

SgxLocalHistogram::~SgxLocalHistogram() {

	free(this->values);

}

void SgxLocalHistogram::computeLocalHistogram() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    //ocall_startHistogramLocalHistogramComputation();
	//enclave::performance::Measurements::startHistogramSgxLocalHistogramComputation();
#endif


	for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
	    values[i] = sgx_calc_sealed_data_size(0, localHistogram[i] * sizeof(uint64_t));
	}



#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	//ocall_stopHistogramLocalHistogramComputation(this->localSize);
	//enclave::performance::Measurements::stopHistogramSgxLocalHistogramComputation(numberOfElements);
#endif

}

uint64_t* SgxLocalHistogram::getLocalHistogram() {

	return this->values;

}

} /* namespace histograms */
} /* namespace enclave */
