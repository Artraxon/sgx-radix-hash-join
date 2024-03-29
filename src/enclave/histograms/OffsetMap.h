/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_HISTOGRAMS_OFFSETMAP_H_
#define HPCJOIN_HISTOGRAMS_OFFSETMAP_H_

#include <stdint.h>

#include <histograms/LocalHistogram.h>
#include <histograms/GlobalHistogram.h>
#include <histograms/AssignmentMap.h>

namespace hpcjoin {
namespace histograms {

class OffsetMap {

public:

	OffsetMap(uint32_t numberOfProcesses, hpcjoin::histograms::LocalHistogram *localHistogram, hpcjoin::histograms::GlobalHistogram *globalHistogram, hpcjoin::histograms::AssignmentMap *assignment);
	~OffsetMap();

public:

	void computeOffsets();

public:

	uint64_t *getBaseOffsets();
	uint64_t *getRelativeWriteOffsets();
	uint64_t *getAbsoluteWriteOffsets();
	uint64_t *getLocalOffsets();
	uint64_t * getLocalHistogram();

protected:

	void computeBaseOffsets();
	void computeRelativePrivateOffsets();
	void computeAbsolutePrivateOffsets();
	void computeLocalOffsets();

protected:

	uint32_t numberOfProcesses;
	hpcjoin::histograms::LocalHistogram *localHistogram;
	hpcjoin::histograms::GlobalHistogram *globalHistogram;
	hpcjoin::histograms::AssignmentMap *assignment;

	uint64_t *baseOffsets;
	uint64_t *relativeWriteOffsets;
	uint64_t *absoluteWriteOffsets;
	uint64_t *localOffsets;


};

} /* namespace histograms */
} /* namespace enclave */

#endif /* HPCJOIN_HISTOGRAMS_OFFSETMAP_H_ */
