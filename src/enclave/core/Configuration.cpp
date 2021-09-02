//
// Created by leonhard on 18.08.21.
//
#include <core/Configuration.h>
#include <core/Parameters.h>
#include <data/CompressedTuple.h>
#include "../../shared/core/Configuration.h"
#include "../../shared/core/Parameters.h"
#include <utils/OcallWrappers.h>

namespace hpcjoin {
    namespace core {
        uint32_t Configuration::CACHELINE_SIZE_BYTES;
        uint32_t Configuration::CACHELINES_PER_MEMORY_BUFFER;
        uint16_t Configuration::MODE;

        uint64_t Configuration::MEMORY_BUFFER_SIZE_BYTES;
        uint64_t Configuration::MEMORY_PARTITION_SIZE_BYTES;


        void Configuration::setupConfig(arguments args){
            Configuration::CACHELINE_SIZE_BYTES = args.cacheline_tuples * sizeof(hpcjoin::data::CompressedTuple);
            Configuration::MODE = (args.package_cachelines == 0 ? 1: 2);
            Configuration::CACHELINES_PER_MEMORY_BUFFER = args.package_cachelines;

            //fprintf(1, "Set Mode to %li", MODE);

            Configuration::MEMORY_BUFFER_SIZE_BYTES = Configuration::CACHELINES_PER_MEMORY_BUFFER * Configuration::CACHELINE_SIZE_BYTES;
            Configuration::MEMORY_PARTITION_SIZE_BYTES = Configuration::MEMORY_BUFFERS_PER_PARTITION * Configuration::MEMORY_BUFFER_SIZE_BYTES;

        }
    }
}
