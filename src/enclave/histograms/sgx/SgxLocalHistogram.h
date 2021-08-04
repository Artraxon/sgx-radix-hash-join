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

	SgxLocalHistogram(hpcjoin::histograms::LocalHistogram *localHistogram);
	~SgxLocalHistogram();

public:

	void computeLocalHistogram();

	uint64_t *getLocalHistogram();

protected:

	hpcjoin::histograms::LocalHistogram *localHistogram;
	uint64_t *values;

};

} /* namespace histograms */
} /* namespace enclave */

#endif /* HPCJOIN_HISTOGRAMS_SGXLOCALHISTOGRAM_H_ */
