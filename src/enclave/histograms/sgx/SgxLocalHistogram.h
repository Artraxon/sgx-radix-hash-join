/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_HISTOGRAMS_SGXLOCALHISTOGRAM_H_
#define HPCJOIN_HISTOGRAMS_SGXLOCALHISTOGRAM_H_

#include <data/Relation.h>
#include <histograms/LocalHistogram.h>

namespace hpcjoin {
namespace histograms {

class SgxLocalHistogram {

public:

	SgxLocalHistogram(uint64_t * localHistogram, uint32_t numberOfNodes, uint64_t localSize);
	~SgxLocalHistogram();

public:

	void computeLocalHistogram();

	uint64_t* getLocalHistogram();

protected:

	uint64_t * localHistogram;
	uint64_t* values;

	uint32_t numberOfNodes;
	uint64_t localSize;
};

} /* namespace histograms */
} /* namespace enclave */

#endif /* HPCJOIN_HISTOGRAMS_SGXLOCALHISTOGRAM_H_ */
