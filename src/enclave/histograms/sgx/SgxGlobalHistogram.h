/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_HISTOGRAMS_GLOBALHISTOGRAM_H_
#define HPCJOIN_HISTOGRAMS_GLOBALHISTOGRAM_H_

#include <histograms/sgx/SgxLocalHistogram.h>

namespace hpcjoin {
namespace histograms {

class SgxGlobalHistogram {

public:

	SgxGlobalHistogram(hpcjoin::histograms::SgxLocalHistogram *localHistogram);
	~SgxGlobalHistogram();

public:

	void computeGlobalHistogram();
	uint64_t *getGlobalHistogram();

protected:

	hpcjoin::histograms::SgxLocalHistogram *localHistogram;
	uint64_t *values;

};

} /* namespace histograms */
} /* namespace enclave */

#endif /* HPCJOIN_HISTOGRAMS_GLOBALHISTOGRAM_H_ */
