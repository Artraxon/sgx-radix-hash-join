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
#include <utils/OcallWrappers.h>
#include <communication/Encryption.h>

#include <core/Configuration.h>
#include <data/CompressedTuple.h>

namespace hpcjoin {
namespace histograms {


#define MODE (hpcjoin::core::Configuration::MODE)
SgxLocalHistogram::SgxLocalHistogram(uint64_t * localHistogram, uint32_t numberOfNodes, uint64_t localSize) {

    this->localSize = localSize;
	this->localHistogram = localHistogram;
	this->numberOfNodes = numberOfNodes;

	this->values = (uint64_t *) calloc(hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, sizeof(uint64_t));
    if(MODE == NOCACHING){
        this->packageValues = values;
    } else {
        this->packageValues = (uint64_t *) calloc((hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT * CACHING),
                                                  sizeof(uint64_t));
    }

}

SgxLocalHistogram::~SgxLocalHistogram() {

	free(this->values);

    if(MODE == CACHING) free(this->packageValues);

}

void SgxLocalHistogram::computeLocalHistogram() {

#ifdef MEASUREMENT_DETAILS_HISTOGRAM
    //ocall_startHistogramLocalHistogramComputation();
	//enclave::performance::Measurements::startHistogramSgxLocalHistogramComputation();
    ocall_startSgxHistogramLocalSgxHistogramComputation();
#endif
    uint64_t totalSize = 0;
    if(MODE == CACHING){
        uint64_t packageSealedSize = communication::Encryption::getEncryptedSize(core::Configuration::MEMORY_BUFFER_SIZE_BYTES);
        printf("datasize: %ld", packageSealedSize);
        const uint64_t tuplesPerPackage = core::Configuration::MEMORY_BUFFER_SIZE_BYTES/ sizeof(data::CompressedTuple);
        for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
            uint64_t packages = (localHistogram[i] / tuplesPerPackage);
            uint64_t packagedSize = packages * packageSealedSize;
            uint64_t restSize = communication::Encryption::getEncryptedSize(localHistogram[i] % tuplesPerPackage * sizeof(data::CompressedTuple));
            values[i] = packagedSize + restSize;
            totalSize += values[i];
            packageValues[i * CACHING] = packages;
            packageValues[i * CACHING + 1] = restSize;
        }
    } else {
        for (uint32_t i = 0; i < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++i) {
            values[i] = communication::Encryption::getEncryptedSize(localHistogram[i] * sizeof(uint64_t));
            totalSize += values[i];
        }
    }



#ifdef MEASUREMENT_DETAILS_HISTOGRAM
	//ocall_stopHistogramLocalHistogramComputation(this->localSize);
	//enclave::performance::Measurements::stopHistogramSgxLocalHistogramComputation(numberOfElements);
    ocall_stopSgxHistogramLocalSgxHistogramComputation(totalSize);
#endif

}

uint64_t* SgxLocalHistogram::getLocalHistogram() {

	return this->values;

}
uint64_t* SgxLocalHistogram::getPackageHistogram() {
    return this->packageValues;
}

} /* namespace histograms */
} /* namespace enclave */
