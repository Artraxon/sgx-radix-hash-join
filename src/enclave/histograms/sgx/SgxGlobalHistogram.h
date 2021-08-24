/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_HISTOGRAMS_SGXGLOBALHISTOGRAM_H_
#define HPCJOIN_HISTOGRAMS_SGXGLOBALHISTOGRAM_H_

#include <histograms/sgx/SgxLocalHistogram.h>
#include <histograms/AssignmentMap.h>

namespace hpcjoin {
namespace histograms {

class SgxGlobalHistogram {

public:

	SgxGlobalHistogram(hpcjoin::histograms::SgxLocalHistogram* localHistogram, hpcjoin::histograms::AssignmentMap* assignmentMap, uint32_t numberOfNodes, uint32_t nodeID);
	~SgxGlobalHistogram();

public:

	void computeGlobalHistogram();
	uint64_t* getGlobalHistogram();
	uint64_t* getSealedSizes();
	uint64_t getSealedSizesCount();

protected:

	hpcjoin::histograms::SgxLocalHistogram* localHistogram;
	hpcjoin::histograms::AssignmentMap* assignmentMap;
    uint64_t* values;
    uint64_t* sealedSizes;
    uint64_t sealedSizesCount;
    uint64_t sealedSizesSize;
    uint32_t numberOfNodes;
    uint32_t nodeID;

protected:

    void prepareSendBuffer(int* sendDisp, uint64_t * sendBuf, uint64_t* data);

};

} /* namespace histograms */
} /* namespace enclave */

#endif /* HPCJOIN_HISTOGRAMS_GLOBALHISTOGRAM_H_ */
