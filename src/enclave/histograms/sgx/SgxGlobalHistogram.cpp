/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "SgxGlobalHistogram.h"
#include "SgxLocalHistogram.h"

#include <mpi.h>

#include <core/Configuration.h>
#include <utils/Debug.h>
#include <Enclave_t.h>
namespace hpcjoin {
namespace histograms {


SgxGlobalHistogram::SgxGlobalHistogram(hpcjoin::histograms::SgxLocalHistogram *localHistogram) {

	this->localHistogram = localHistogram;
	this->values = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));

}

SgxGlobalHistogram::~SgxGlobalHistogram() {

	free(values);

}

void SgxGlobalHistogram::computeGlobalHistogram() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    ocall_startHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::startHistogramSgxGlobalHistogramComputation();
#endif
    ocall_MPI_allreduce_sum(this->localHistogram->getLocalHistogram(), this->values, hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT);

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	ocall_stopHistogramGlobalHistogramComputation();
	//enclave::performance::Measurements::stopHistogramSgxGlobalHistogramComputation();
#endif

}

uint64_t* SgxGlobalHistogram::getGlobalHistogram() {

	return this->values;

}


} /* namespace histograms */
} /* namespace enclave */
