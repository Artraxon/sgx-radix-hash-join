/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <Enclave_t.h>
#include <operators/HashJoin.h>
#include <core/Configuration.h>
#include "../shared/utils/Debug.h"
#include <tasks/BuildProbe.h>
#include <memory/Pool.h>
#include <utils/OcallWrappers.h>


void ecall_start_hash_join(arguments* passed_args){

    //Save parameters in global variable
    hpcjoin::core::Configuration::setupConfig(*passed_args);

    JOIN_MEM_DEBUG("Main Start");

	JOIN_DEBUG("Main", "Initializing MPI");

#ifdef USE_FOMPI
	foMPI_Init(NULL, NULL);
#endif

	JOIN_DEBUG("Main", "Gathering deployment information");

	int32_t numberOfNodes = -1;
	int32_t nodeId = -1;

	ocall_MPI_Comm_size(&numberOfNodes);
	ocall_MPI_Comm_rank(&nodeId);

	JOIN_DEBUG("Main", "Node %d is preparing performance counters", nodeId);

	ocall_init(nodeId, numberOfNodes, "experiment");
	//hpcjoin::performance::Measurements::init(nodeId, numberOfNodes, "experiment");

	ocall_writeMetaData_int("NUMNODES", numberOfNodes);
    ocall_writeMetaData_int("NODEID", nodeId);
	//hpcjoin::performance::Measurements::writeMetaData("NUMNODES", numberOfNodes);
	//hpcjoin::performance::Measurements::writeMetaData("NODEID", nodeId);

	char hostname[1024];
	memset(hostname, 0, 1024);
	ocall_gethostname(hostname, 1023);
	JOIN_DEBUG("main", "Node %d on machine %s\n", nodeId, hostname);
    ocall_writeMetaData("HOST", hostname);
	//hpcjoin::performance::Measurements::writeMetaData("HOST", hostname);

	JOIN_ASSERT(numberOfNodes > 0, "Main", "Number of nodes not set");
	JOIN_ASSERT(nodeId >= 0, "Main", "Node id not set");
	JOIN_ASSERT(nodeId < numberOfNodes, "Main", "Node id is not in range");

	JOIN_DEBUG("Main", "Node %d is loading relations", nodeId);

	JOIN_MEM_DEBUG("Init Completed");

/*
	uint64_t globalInnerRelationSize = ((uint64_t) numberOfNodes) * 200000;
	uint64_t globalOuterRelationSize = ((uint64_t) numberOfNodes) * 200000;
*/

    uint64_t globalInnerRelationSize = ((uint64_t) numberOfNodes) * passed_args->tuples_per_node;
    uint64_t globalOuterRelationSize = ((uint64_t) numberOfNodes) * passed_args->tuples_per_node;

    uint64_t localInnerRelationSize =
			(nodeId < numberOfNodes - 1) ? (globalInnerRelationSize / numberOfNodes) : (globalInnerRelationSize - (numberOfNodes - 1) * (globalInnerRelationSize / numberOfNodes));

	uint64_t localOuterRelationSize =
			(nodeId < numberOfNodes - 1) ? (globalOuterRelationSize / numberOfNodes) : (globalOuterRelationSize - (numberOfNodes - 1) * (globalOuterRelationSize / numberOfNodes));

	ocall_writeMetaData_int("GISZ", globalInnerRelationSize);
	//hpcjoin::performance::Measurements::writeMetaData("GISZ", globalInnerRelationSize);
	ocall_writeMetaData_int("GOSZ", globalOuterRelationSize);
	//hpcjoin::performance::Measurements::writeMetaData("GOSZ", globalOuterRelationSize);
	ocall_writeMetaData_int("LISZ", localInnerRelationSize);
	//hpcjoin::performance::Measurements::writeMetaData("LISZ", localInnerRelationSize);
	ocall_writeMetaData_int("LOSZ", localOuterRelationSize);
	//hpcjoin::performance::Measurements::writeMetaData("LOSZ", localOuterRelationSize);

	hpcjoin::memory::Pool::allocate(hpcjoin::core::Configuration::ALLOCATION_FACTOR * (localInnerRelationSize+localOuterRelationSize) * sizeof(hpcjoin::data::Tuple));
	hpcjoin::data::Relation *innerRelation = new hpcjoin::data::Relation(localInnerRelationSize, globalInnerRelationSize);
	hpcjoin::data::Relation *outerRelation = new hpcjoin::data::Relation(localOuterRelationSize, globalOuterRelationSize);

	JOIN_MEM_DEBUG("Relations created");

	ocall_srand(1234+nodeId);
	innerRelation->fillUniqueValues(nodeId * (globalInnerRelationSize / numberOfNodes), nodeId * (globalInnerRelationSize / numberOfNodes));
	outerRelation->fillUniqueValues((numberOfNodes - nodeId - 1) * (globalOuterRelationSize / numberOfNodes), nodeId * (globalOuterRelationSize / numberOfNodes));
	//outerRelation->fillModuloValues((numberOfNodes - nodeId - 1) * (globalInnerRelationSize / numberOfNodes), nodeId * (globalOuterRelationSize / numberOfNodes), innerRelation->getLocalSize());

	if (numberOfNodes > 1) {
		innerRelation->distribute(nodeId, numberOfNodes);
		outerRelation->distribute(nodeId, numberOfNodes);
	}

	//innerRelation->debugKeyPrint();
	//outerRelation->debugKeyPrint();

	JOIN_MEM_DEBUG("Relations distributed");

	JOIN_DEBUG("Main", "Node %d is preparing join", nodeId);

	hpcjoin::operators::HashJoin *hashJoin = new hpcjoin::operators::HashJoin(numberOfNodes, nodeId, innerRelation, outerRelation);
	JOIN_MEM_DEBUG("Join created");

	ocall_MPI_Barrier();

	JOIN_DEBUG("Main", "Node %d is starting join", nodeId);

	JOIN_MEM_DEBUG("Join Start");
	hashJoin->join();
	JOIN_MEM_DEBUG("Join Stop");

	JOIN_DEBUG("Main", "Node %d finished join", nodeId);

	ocall_MPI_Barrier();

	for (std::map<uint64_t , uint16_t>::iterator it = hpcjoin::tasks::BuildProbe::joinHistogram.begin(); it != hpcjoin::tasks::BuildProbe::joinHistogram.end(); ++it)
	{
        int k = 0;
	}


	JOIN_DEBUG("Main", "Node %d finalizing measurements", nodeId);

	if (nodeId != hpcjoin::core::Configuration::RESULT_AGGREGATION_NODE) {
	    ocall_sendMeasurementsToAggregator();
		//hpcjoin::performance::Measurements::sendMeasurementsToAggregator();
	} else {
	    ocall_setResultCounter(hashJoin->RESULT_COUNTER);
	    ocall_printMeasurements(numberOfNodes, nodeId);
		//hpcjoin::performance::Measurements::printMeasurements(numberOfNodes, nodeId);
	}
	ocall_storeAllMeasurements();
	//hpcjoin::performance::Measurements::storeAllMeasurements();

	delete hashJoin;
	// OPTIMIZATION innerRelation deleted during join
	// OPTIMIZATION outerRelation deleted during join

#ifdef USE_FOMPI
	foMPI_Finalize();
#endif

	ocall_MPI_Finalize();

}

