/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "HashJoin.h"

#include <stdlib.h>

#include <data/Window.h>
#include <core/Configuration.h>
#include <tasks/HistogramComputation.h>
#include <tasks/SgxHistogramComputation.h>
#include <tasks/SgxPartitioning.h>
#include <tasks/NetworkPartitioning.h>
#include <tasks/LocalPartitioning.h>
#include <tasks/BuildProbe.h>
#include <utils/Debug.h>
#include <memory/Pool.h>
#include <data/CompressedTuple.h>

#include <Enclave_t.h>

namespace hpcjoin {
namespace operators {

uint64_t HashJoin::RESULT_COUNTER = 0;
std::queue<hpcjoin::tasks::Task *> HashJoin::TASK_QUEUE;

HashJoin::HashJoin(uint32_t numberOfNodes, uint32_t nodeId, hpcjoin::data::Relation *innerRelation, hpcjoin::data::Relation *outerRelation) {

	this->nodeId = nodeId;
	this->numberOfNodes = numberOfNodes;
	this->innerRelation = innerRelation;
	this->outerRelation = outerRelation;

}

HashJoin::~HashJoin() {

}

void HashJoin::join() {

	/**********************************************************************/

	ocall_MPI_Barrier();
	ocall_startJoin();
	//hpcjoin::performance::Measurements::startJoin();

	/**********************************************************************/

	/**
	 * Histogram computation
	 */

	ocall_startHistogramComputation();
	//hpcjoin::performance::Measurements::startHistogramComputation();
	hpcjoin::tasks::HistogramComputation *histogramComputation = new hpcjoin::tasks::HistogramComputation(this->numberOfNodes, this->nodeId, this->innerRelation,
			this->outerRelation);
	histogramComputation->execute();
    hpcjoin::tasks::SgxHistogramComputation *sgxHistogramComputation
            = new hpcjoin::tasks::SgxHistogramComputation(this->numberOfNodes, this->nodeId,
                                                          histogramComputation->getInnerRelationLocalHistogram(), histogramComputation->getOuterRelationLocalHistogram(),
                                                          innerRelation->getLocalSize(), outerRelation->getLocalSize(),
                                                          histogramComputation->getAssignmentMap());
    sgxHistogramComputation->execute();
	ocall_stopHistogramComputation();
	//hpcjoin::performance::Measurements::stopHistogramComputation();
	JOIN_MEM_DEBUG("Histogram phase completed");

	/**********************************************************************/

	/**
	 * Window allocation
	 */

	ocall_startWindowAllocation();
	//hpcjoin::performance::Measurements::startWindowAllocation();
	hpcjoin::data::Window *innerWindow = new hpcjoin::data::Window(this->numberOfNodes, this->nodeId, histogramComputation->getAssignmentMap(),
			histogramComputation->getInnerRelationLocalHistogram(), histogramComputation->getInnerRelationGlobalHistogram(), histogramComputation->getInnerRelationBaseOffsets(),
			histogramComputation->getInnerRelationWriteOffsets(),
			sgxHistogramComputation->getInnerRelationSgxLocalHistogram(), sgxHistogramComputation->getInnerRelationSgxGlobalHistogram(),
			sgxHistogramComputation->getInnerRelationBaseOffsets(), sgxHistogramComputation->getInnerRelationWriteOffsets(),
			sgxHistogramComputation->getInnerSealedSizes());

	hpcjoin::data::Window *outerWindow = new hpcjoin::data::Window(this->numberOfNodes, this->nodeId, histogramComputation->getAssignmentMap(),
			histogramComputation->getOuterRelationLocalHistogram(), histogramComputation->getOuterRelationGlobalHistogram(), histogramComputation->getOuterRelationBaseOffsets(),
			histogramComputation->getOuterRelationWriteOffsets(),
			sgxHistogramComputation->getOuterRelationSgxLocalHistogram(), sgxHistogramComputation->getOuterRelationSgxGlobalHistogram(),
			sgxHistogramComputation->getOuterRelationBaseOffsets(), sgxHistogramComputation->getOuterRelationWriteOffsets(),
			sgxHistogramComputation->getOuterSealedSizes());

	ocall_stopWindowAllocation();
	//hpcjoin::performance::Measurements::stopWindowAllocation();
	JOIN_MEM_DEBUG("Window allocated");

	/**********************************************************************/

	/**
	 * Network partitioning
	 */

	ocall_startNetworkPartitioning();
	//hpcjoin::performance::Measurements::startNetworkPartitioning();
	hpcjoin::tasks::SgxPartitioning *networkPartitioning = new hpcjoin::tasks::SgxPartitioning(this->nodeId, this->innerRelation, this->outerRelation, innerWindow,
			outerWindow, histogramComputation->getInnerLocalOffsets(), histogramComputation->getOuterLocalOffsets());
	networkPartitioning->execute();
	ocall_stopNetworkPartitioning();
	//hpcjoin::performance::Measurements::stopNetworkPartitioning();
	JOIN_MEM_DEBUG("Network phase completed");

	// OPTIMIZATION Save memory as soon as possible
	//delete this->innerRelation;
	//delete this->outerRelation;
	JOIN_MEM_DEBUG("Input relations deleted");

	/**********************************************************************/

	/**
	 * Main synchronization
	 */

	ocall_startWaitingForNetworkCompletion();
	//hpcjoin::performance::Measurements::startWaitingForNetworkCompletion();
	ocall_MPI_Barrier();
	ocall_stopWaitingForNetworkCompletion();
	//hpcjoin::performance::Measurements::stopWaitingForNetworkCompletion();

	/**********************************************************************/

	/**
	 * Unseal Data
	 */

	 innerWindow->unsealData();
	 outerWindow->unsealData();

	/**
	 * Prepare transition
	 */

	ocall_startLocalProcessingPreparations();
	//hpcjoin::performance::Measurements::startLocalProcessingPreparations();
	if (hpcjoin::core::Configuration::ENABLE_TWO_LEVEL_PARTITIONING) {
		//enclave::memory::Pool::allocate((innerWindow->computeLocalWindowSize() + outerWindow->computeLocalWindowSize())*sizeof(enclave::data::Tuple));
		hpcjoin::memory::Pool::reset();
	}
	// Create initial set of tasks
	uint32_t *assignment = histogramComputation->getAssignment();
	for (uint32_t p = 0; p < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++p) {
		if (assignment[p] == this->nodeId) {
			hpcjoin::data::CompressedTuple *innerRelationPartition = innerWindow->getPartition(p);
			uint64_t innerRelationPartitionSize = innerWindow->getPartitionSize(p);
			hpcjoin::data::CompressedTuple *outerRelationPartition = outerWindow->getPartition(p);
			uint64_t outerRelationPartitionSize = outerWindow->getPartitionSize(p);

			if (hpcjoin::core::Configuration::ENABLE_TWO_LEVEL_PARTITIONING) {
				TASK_QUEUE.push(new hpcjoin::tasks::LocalPartitioning(innerRelationPartitionSize, innerRelationPartition, outerRelationPartitionSize, outerRelationPartition));
			} else {
				TASK_QUEUE.push(new hpcjoin::tasks::BuildProbe(innerRelationPartitionSize, innerRelationPartition, outerRelationPartitionSize, outerRelationPartition));
			}
		}
	}

	// Delete the network related computation
	delete histogramComputation;
	delete networkPartitioning;

	JOIN_MEM_DEBUG("Local phase prepared");

	ocall_stopLocalProcessingPreparations();
	//hpcjoin::performance::Measurements::stopLocalProcessingPreparations();

	/**********************************************************************/

	/**
	 * Local processing
	 */

	// OPTIMIZATION Delete window as soon as possible
	bool windowsDeleted = false;

	// Execute tasks
	ocall_startLocalProcessing();
	//hpcjoin::performance::Measurements::startLocalProcessing();
	while (TASK_QUEUE.size() > 0) {

		hpcjoin::tasks::Task *task = TASK_QUEUE.front();
		TASK_QUEUE.pop();

		// OPTIMIZATION When second partitioning pass is completed, windows are no longer required
		if (hpcjoin::core::Configuration::ENABLE_TWO_LEVEL_PARTITIONING && windowsDeleted) {
			if (task->getType() == TASK_BUILD_PROBE) {
				delete innerWindow;
				delete outerWindow;
				windowsDeleted = true;
			}
		}

		task->execute();
		delete task;

	}
	ocall_stopLocalProcessing();
	//hpcjoin::performance::Measurements::stopLocalProcessing();

	JOIN_MEM_DEBUG("Local phase completed");

	/**********************************************************************/

	ocall_stopJoin();
	//hpcjoin::performance::Measurements::stopJoin();

	// OPTIMIZATION (see above)
	if (!windowsDeleted) {
		delete innerWindow;
		delete outerWindow;
	}

}

} /* namespace operators */
} /* namespace enclave */

void ecall_getResultCounter(uint64_t* counter){
    *counter = hpcjoin::operators::HashJoin::RESULT_COUNTER;
}
