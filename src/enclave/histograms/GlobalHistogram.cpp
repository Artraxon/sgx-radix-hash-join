/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "GlobalHistogram.h"


#include <core/Configuration.h>
#include <utils/Debug.h>
#include <Enclave_t.h>
namespace hpcjoin {
namespace histograms {


GlobalHistogram::GlobalHistogram(LocalHistogram* localHistogram) {

	this->localHistogram = localHistogram;
	this->values = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));

}

GlobalHistogram::~GlobalHistogram() {

	free(values);

}

void GlobalHistogram::computeGlobalHistogram() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    ocall_startHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::startHistogramGlobalHistogramComputation();
#endif

	ocall_MPI_allreduce_sum(this->localHistogram->getLocalHistogram(), this->values, hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT);

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	ocall_stopHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::stopHistogramGlobalHistogramComputation();
#endif

}

uint64_t* GlobalHistogram::getGlobalHistogram() {

	return this->values;

}


} /* namespace histograms */
} /* namespace enclave */
