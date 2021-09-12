/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#include "SgxPartitioning.h"

#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <core/Configuration.h>
#include <data/CompressedTuple.h>
#include <utils/Debug.h>
#include <histograms/LocalHistogram.h>
#include <sgx_tseal.h>
#include <Enclave_t.h>
#include <histograms/OffsetMap.h>
#include <histograms/sgx/SgxOffsetMap.h>

/*
#ifndef NETWORK_PARTITIONING_CACHELINE_SIZE
#define NETWORK_PARTITIONING_CACHELINE_SIZE (64)
#endif
*/

#define TUPLES_PER_CACHELINE (NETWORK_PARTITIONING_CACHELINE_SIZE / sizeof(hpcjoin::data::CompressedTuple))

#define HASH_BIT_MODULO(KEY, MASK, NBITS) (((KEY) & (MASK)) >> (NBITS))

#define PARTITION_ACCESS(p) (((char *) inMemoryBuffer) + (p * hpcjoin::core::Configuration::MEMORY_PARTITION_SIZE_BYTES))

namespace hpcjoin {
namespace tasks {

typedef union {

	struct {
		hpcjoin::data::CompressedTuple tuples[TUPLES_PER_CACHELINE];
	} tuples;

	struct {
		hpcjoin::data::CompressedTuple tuples[TUPLES_PER_CACHELINE - 1];
		uint32_t inCacheCounter;
		uint32_t memoryCounter;
	} data;

} cacheline_t;

//Doesn't need SgxOffsets since the Window Class takes care of that
SgxPartitioning::SgxPartitioning(uint32_t nodeId, hpcjoin::data::Relation* innerRelation, hpcjoin::data::Relation* outerRelation, hpcjoin::data::Window* innerWindow,
                                 hpcjoin::data::Window* outerWindow, uint64_t * innerOffsets, uint64_t * outerOffsets) {

	this->nodeId = nodeId;

	this->innerRelation = innerRelation;
	this->outerRelation = outerRelation;

	this->innerWindow = innerWindow;
	this->outerWindow = outerWindow;

	this->innerOffsets = innerOffsets;
    this->outerOffsets = outerOffsets;

	JOIN_ASSERT(hpcjoin::core::Configuration::CACHELINE_SIZE_BYTES == NETWORK_PARTITIONING_CACHELINE_SIZE, "Network Partitioning", "Cache line sizes do not match. This is a hack and the value needs to be edited in two places.");

}

SgxPartitioning::~SgxPartitioning() {
}

void SgxPartitioning::execute() {

	JOIN_DEBUG("Network Partitioning", "Node %d is partitioning inner relation", this->nodeId);
	partition(innerRelation, innerWindow, innerOffsets);

	JOIN_DEBUG("Network Partitioning", "Node %d is partitioning outer relation", this->nodeId);
	partition(outerRelation, outerWindow, outerOffsets);

}

void SgxPartitioning::partition(hpcjoin::data::Relation *relation,
                                hpcjoin::data::Window *window,
                                uint64_t * offsets){

    window->start();

	uint64_t const numberOfElements = relation->getLocalSize();
	hpcjoin::data::Tuple * const data = relation->getData();
	uint64_t localCounter[hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT];
	std::fill(localCounter, localCounter + hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT, 0);

	// Create in-memory buffer
	uint64_t const bufferedPartitionCount = hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT;
	uint64_t const bufferedPartitionSize = hpcjoin::core::Configuration::MEMORY_PARTITION_SIZE_BYTES;
	uint64_t const inMemoryBufferSize = bufferedPartitionCount * bufferedPartitionSize;

	const uint32_t partitionBits = hpcjoin::core::Configuration::NETWORK_PARTITIONING_FANOUT;

#ifdef MEASUREMENT_DETAILS_NETWORK
	ocall_startNetworkPartitioningMemoryAllocation();
#endif

	hpcjoin::data::CompressedTuple* inMemoryBuffer = (hpcjoin::data::CompressedTuple *) calloc(offsets[hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT],
                                            sizeof(hpcjoin::data::CompressedTuple));

    JOIN_ASSERT(inMemoryBuffer != NULL, "Network Partitioning", "Could not allocate in-memory buffer");


#ifdef MEASUREMENT_DETAILS_NETWORK
	ocall_stopNetworkPartitioningMemoryAllocation(inMemoryBufferSize);
    ocall_startNetworkPartitioningMainPartitioning();
#endif
	JOIN_DEBUG("Network Partitioning", "Node %d is setting counter to zero", this->nodeId);

	for (uint64_t i = 0; i < numberOfElements; ++i) {

		// Compute partition
		uint32_t partitionId = HASH_BIT_MODULO(data[i].key, hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT - 1, 0);


		// Move data to cache line
		hpcjoin::data::CompressedTuple* cacheLine = (hpcjoin::data::CompressedTuple *) (inMemoryBuffer + offsets[partitionId] + localCounter[partitionId]);
		//cacheLine[inCacheCounter] = data[i];
		cacheLine->value = data[i].rid + ((data[i].key >> partitionBits) << (partitionBits + hpcjoin::core::Configuration::PAYLOAD_BITS));
		++localCounter[partitionId];
	}

#ifdef MEASUREMENT_DETAILS_NETWORK
    ocall_stopNetworkPartitioningMainPartitioning(numberOfElements);
    ocall_startNetworkPartitioningFlushPartitioning();
#endif

    JOIN_DEBUG("Network Partitioning", "Node %d is flushing remaining tuples", this->nodeId);

    uint64_t currentOffset = 0;
    for (int p = 0; p < hpcjoin::core::Configuration::NETWORK_PARTITIONING_COUNT; ++p) {
        hpcjoin::data::CompressedTuple* currentPartition = ((hpcjoin::data::CompressedTuple *) inMemoryBuffer) + currentOffset;
        window->write(p, currentPartition, localCounter[p], false);
        currentOffset += localCounter[p];
    }

	window->flush();
	window->stop();

	free(inMemoryBuffer);

#ifdef MEASUREMENT_DETAILS_NETWORK
	ocall_stopNetworkPartitioningFlushPartitioning();
#endif

	window->assertAllTuplesWritten();

}


task_type_t SgxPartitioning::getType() {
	return TASK_NET_PARTITION;
}

} /* namespace tasks */
} /* namespace enclave */

