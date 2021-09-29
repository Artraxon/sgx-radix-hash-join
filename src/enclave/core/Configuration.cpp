//
// Created by leonhard on 18.08.21.
//
#include <core/Configuration.h>
#include <core/Parameters.h>
#include <data/CompressedTuple.h>
#include "../../shared/core/Configuration.h"
#include "../../shared/core/Parameters.h"
#include <utils/OcallWrappers.h>

/*
#ifndef LOCAL_PARTITIONING_CACHELINE_SIZE
#define LOCAL_PARTITIONING_CACHELINE_SIZE (64)
#endif
*/
namespace hpcjoin {
    namespace core {
        uint32_t Configuration::CACHELINE_SIZE_BYTES;
        uint32_t Configuration::CACHELINES_PER_MEMORY_BUFFER;
        uint16_t Configuration::MODE;
        uint64_t Configuration::ZIPF_SIZE;
        double Configuration::ZIPF_FACTOR;

        uint64_t Configuration::MEMORY_BUFFER_SIZE_BYTES;
        uint64_t Configuration::MEMORY_PARTITION_SIZE_BYTES;

        uint64_t Configuration::NETWORK_PARTITIONING_FANOUT;
        uint64_t Configuration::LOCAL_PARTITIONING_FANOUT;

        uint64_t Configuration::NETWORK_PARTITIONING_COUNT;
        uint64_t Configuration::LOCAL_PARTITIONING_COUNT;

        void Configuration::setupConfig(arguments args){
            Configuration::CACHELINE_SIZE_BYTES = LOCAL_PARTITIONING_CACHELINE_SIZE;
            Configuration::MODE = (args.package_cachelines == 0 ? 1: 2);
            Configuration::CACHELINES_PER_MEMORY_BUFFER = args.package_cachelines;
            Configuration::ZIPF_FACTOR = args.zipf_factor;
            Configuration::ZIPF_SIZE = args.zipf_size;

            //fprintf(1, "Set Mode to %li", MODE);

            Configuration::MEMORY_BUFFER_SIZE_BYTES = Configuration::CACHELINES_PER_MEMORY_BUFFER * Configuration::CACHELINE_SIZE_BYTES;
            Configuration::MEMORY_PARTITION_SIZE_BYTES = Configuration::MEMORY_BUFFERS_PER_PARTITION * Configuration::MEMORY_BUFFER_SIZE_BYTES;

            NETWORK_PARTITIONING_FANOUT = args.network_fanout;
            LOCAL_PARTITIONING_FANOUT = args.local_fanout;
            NETWORK_PARTITIONING_COUNT = (1 << NETWORK_PARTITIONING_FANOUT);
            LOCAL_PARTITIONING_COUNT = (1 << LOCAL_PARTITIONING_FANOUT);
        }
    }
}
