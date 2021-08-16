/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_HISTOGRAMS_SGXOFFSETMAP_H_
#define HPCJOIN_HISTOGRAMS_SGXOFFSETMAP_H_

#include <stdint.h>

#include <histograms/sgx/SgxLocalHistogram.h>
#include <histograms/sgx/SgxGlobalHistogram.h>
#include <histograms/AssignmentMap.h>

namespace hpcjoin {
namespace histograms {

class SgxOffsetMap {

public:

	SgxOffsetMap(uint32_t numberOfProcesses, hpcjoin::histograms::SgxLocalHistogram *localHistogram, hpcjoin::histograms::SgxGlobalHistogram *globalHistogram, hpcjoin::histograms::AssignmentMap *assignment);
	~SgxOffsetMap();

public:

	void computeOffsets();

public:

	uint64_t *getBaseOffsets();
	uint64_t *getRelativeWriteOffsets();
	uint64_t *getAbsoluteWriteOffsets();
	uint64_t * getLocalHistogram();

protected:

	void computeBaseOffsets();
	void computeRelativePrivateOffsets();
	void computeAbsolutePrivateOffsets();

protected:

	uint32_t numberOfProcesses;
	hpcjoin::histograms::SgxLocalHistogram *localHistogram;
	hpcjoin::histograms::SgxGlobalHistogram *globalHistogram;
	hpcjoin::histograms::AssignmentMap *assignment;

	uint64_t *baseOffsets;
	uint64_t *relativeWriteOffsets;
	uint64_t *absoluteWriteOffsets;


};

} /* namespace histograms */
} /* namespace enclave */

#endif /* HPCJOIN_HISTOGRAMS_OFFSETMAP_H_ */
