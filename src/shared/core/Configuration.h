/**
 * @author  Claude Barthels <claudeb@inf.ethz.ch>
 * (c) 2016, ETH Zurich, Systems Group
 *
 */

#ifndef HPCJOIN_CORE_CONFIGURATION_H_
#define HPCJOIN_CORE_CONFIGURATION_H_

#include <stdint.h>
#include <core/Parameters.h>

namespace hpcjoin {
namespace core {

class Configuration {

public:

	static const uint32_t RESULT_AGGREGATION_NODE = 0;

	static uint32_t CACHELINE_SIZE_BYTES;
	static uint32_t CACHELINES_PER_MEMORY_BUFFER;
	static const uint32_t MEMORY_BUFFERS_PER_PARTITION = 2;

	static uint16_t MODE;
    static uint64_t ZIPF_SIZE;
    static double  ZIPF_FACTOR;

	static uint64_t MEMORY_BUFFER_SIZE_BYTES;
	static uint64_t MEMORY_PARTITION_SIZE_BYTES;

	static const bool ENABLE_TWO_LEVEL_PARTITIONING = true;

	static uint64_t NETWORK_PARTITIONING_FANOUT;
	static uint64_t LOCAL_PARTITIONING_FANOUT;

	static uint64_t NETWORK_PARTITIONING_COUNT;
	static uint64_t LOCAL_PARTITIONING_COUNT;

	static constexpr double ALLOCATION_FACTOR = 1.1;

	static const uint32_t PAYLOAD_BITS = 27;

    static void setupConfig(arguments args);
};

} /* namespace core */
} /* namespace enclave */


#endif /* HPCJOIN_CORE_CONFIGURATION_H_ */
