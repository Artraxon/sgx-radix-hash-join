/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_TASKS_SGXHISTOGRAMCOMPUTATION_H_
#define HPCJOIN_TASKS_SGXHISTOGRAMCOMPUTATION_H_

#include <tasks/Task.h>
#include <data/Relation.h>
#include <histograms/GlobalHistogram.h>
#include <histograms/LocalHistogram.h>
#include <histograms/AssignmentMap.h>
#include <histograms/sgx/SgxOffsetMap.h>

#include <histograms/sgx/SgxLocalHistogram.h>
#include <histograms/sgx/SgxGlobalHistogram.h>

namespace hpcjoin {
namespace tasks {

class SgxHistogramComputation : public Task {

public:

    SgxHistogramComputation(uint32_t numberOfNodes, uint32_t nodeId,
                            uint64_t * innerLocalHistogram, uint64_t * outerLocalHistogram,
                            uint64_t innerRelationSize, uint64_t outerRelationSize,
                            hpcjoin::histograms::AssignmentMap* assignmentMap);

    ~SgxHistogramComputation();

public:

	void execute();
	task_type_t getType();

protected:

	void computeLocalHistograms();
	void computeGlobalInformation();

public:

	uint32_t *getAssignment();
	uint64_t *getInnerRelationSgxLocalHistogram();
	uint64_t *getOuterRelationSgxLocalHistogram();
	uint64_t *getInnerRelationSgxGlobalHistogram();
	uint64_t *getOuterRelationSgxGlobalHistogram();
	uint64_t *getInnerRelationBaseOffsets();
	uint64_t *getOuterRelationBaseOffsets();
	uint64_t *getInnerRelationWriteOffsets();
	uint64_t *getOuterRelationWriteOffsets();


    uint64_t *getInnerSealedSizes();
    uint64_t *getOuterSealedSizes();
protected:

	uint32_t nodeId;
	uint32_t numberOfNodes;

	uint64_t innerRelationSize;
    uint64_t outerRelationSize;

	uint64_t *innerRelationLocalHistogram;
	uint64_t *outerRelationLocalHistogram;

    hpcjoin::histograms::SgxLocalHistogram *innerRelationSgxLocalHistogram;
    hpcjoin::histograms::SgxLocalHistogram *outerRelationSgxLocalHistogram;

	hpcjoin::histograms::SgxGlobalHistogram* innerRelationSgxGlobalHistogram;
	hpcjoin::histograms::SgxGlobalHistogram* outerRelationSgxGlobalHistogram;

	hpcjoin::histograms::AssignmentMap *assignment;

	hpcjoin::histograms::SgxOffsetMap *innerOffsets;
	hpcjoin::histograms::SgxOffsetMap *outerOffsets;

};

} /* namespace tasks */
} /* namespace enclave */

#endif /* HPCJOIN_TASKS_SGXHISTOGRAMCOMPUTATION_H_ */
