/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "SgxHistogramComputation.h"

#include <stdlib.h>

#include <core/Configuration.h>
#include <utils/Debug.h>
#include <histograms/LocalHistogram.h>
#include <histograms/AssignmentMap.h>

#include <histograms/sgx/SgxLocalHistogram.h>
#include <histograms/sgx/SgxGlobalHistogram.h>

namespace hpcjoin {
namespace tasks {

    SgxHistogramComputation::SgxHistogramComputation(uint32_t numberOfNodes, uint32_t nodeId,
                                                     uint64_t * innerLocalHistogram, uint64_t * outerLocalHistogram,
                                                     uint64_t innerRelationSize, uint64_t outerRelationSize,
                                                     hpcjoin::histograms::AssignmentMap* assignmentMap) {

	this->nodeId = nodeId;
	this->numberOfNodes = numberOfNodes;

    this->assignment = assignmentMap;

    this->innerRelationSize = innerRelationSize;
    this->outerRelationSize = outerRelationSize;

    this->innerRelationLocalHistogram = innerLocalHistogram;
	this->outerRelationLocalHistogram = outerLocalHistogram;

	this->innerRelationSgxLocalHistogram = new hpcjoin::histograms::SgxLocalHistogram(innerLocalHistogram, numberOfNodes, innerRelationSize);
	this->outerRelationSgxLocalHistogram = new hpcjoin::histograms::SgxLocalHistogram(outerLocalHistogram, numberOfNodes, outerRelationSize);

	this->innerRelationSgxGlobalHistogram = new hpcjoin::histograms::SgxGlobalHistogram(innerRelationSgxLocalHistogram, assignment, numberOfNodes, nodeID);
    this->outerRelationSgxGlobalHistogram = new hpcjoin::histograms::SgxGlobalHistogram(outerRelationSgxLocalHistogram, assignment, numberOfNodes, nodeID);

	this->innerOffsets = new hpcjoin::histograms::SgxOffsetMap(this->numberOfNodes, this->innerRelationSgxLocalHistogram, this->innerRelationSgxGlobalHistogram, this->assignment);
	this->outerOffsets = new hpcjoin::histograms::SgxOffsetMap(this->numberOfNodes, this->outerRelationSgxLocalHistogram, this->outerRelationSgxGlobalHistogram, this->assignment);

}

SgxHistogramComputation::~SgxHistogramComputation() {

    delete this->innerOffsets;
    delete this->outerOffsets;

    delete this->innerRelationSgxLocalHistogram;
    delete this->outerRelationSgxLocalHistogram;

    delete this->innerRelationSgxGlobalHistogram;
    delete this->outerRelationSgxGlobalHistogram;

}

void SgxHistogramComputation::execute() {

    this->assignment->computePartitionAssignment();

	this->innerRelationSgxLocalHistogram->computeLocalHistogram();
	this->outerRelationSgxLocalHistogram->computeLocalHistogram();

	this->innerRelationSgxGlobalHistogram->computeGlobalHistogram();
	this->outerRelationSgxGlobalHistogram->computeGlobalHistogram();

	this->innerOffsets->computeOffsets();
	this->outerOffsets->computeOffsets();

}


uint32_t* SgxHistogramComputation::getAssignment() {

	return this->assignment->getPartitionAssignment();

}

uint64_t* SgxHistogramComputation::getInnerRelationSgxLocalHistogram() {

	return this->innerRelationSgxLocalHistogram->getLocalHistogram();

}

uint64_t* SgxHistogramComputation::getOuterRelationSgxLocalHistogram() {

	return this->outerRelationSgxLocalHistogram->getLocalHistogram();

}

uint64_t* SgxHistogramComputation::getInnerRelationSgxGlobalHistogram() {

	return this->innerRelationSgxGlobalHistogram->getGlobalHistogram();

}

uint64_t* SgxHistogramComputation::getOuterRelationSgxGlobalHistogram() {

	return this->outerRelationSgxGlobalHistogram->getGlobalHistogram();

}

uint64_t* SgxHistogramComputation::getInnerRelationBaseOffsets() {

	return this->innerOffsets->getBaseOffsets();

}

uint64_t* SgxHistogramComputation::getOuterRelationBaseOffsets() {

	return this->outerOffsets->getBaseOffsets();

}


uint64_t* SgxHistogramComputation::getInnerRelationWriteOffsets() {

	return this->innerOffsets->getAbsoluteWriteOffsets();

}


uint64_t* SgxHistogramComputation::getOuterRelationWriteOffsets() {

	return this->outerOffsets->getAbsoluteWriteOffsets();

}

task_type_t SgxHistogramComputation::getType() {
	return TASK_HISTOGRAM;
}

uint64_t* SgxHistogramComputation::getInnerSealedSizes(){
    return this->innerRelationSgxGlobalHistogram->getSealedSizes();
}

uint64_t* SgxHistogramComputation::getOuterSealedSizes(){
        return this->outerRelationSgxGlobalHistogram->getSealedSizes();
}

} /* namespace tasks */
} /* namespace enclave */


