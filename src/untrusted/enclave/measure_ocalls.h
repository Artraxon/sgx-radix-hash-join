//
// Created by leonhard on 13.07.21.
//

#ifndef RADIX_HASH_JOIN_MEASURE_OCALLS_H
#define RADIX_HASH_JOIN_MEASURE_OCALLS_H
/**
/**
 * Timings for section
 *

void ocall_startJoin();
void ocall_stopJoin();
void ocall_startHistogramComputation();
void ocall_stopHistogramComputation();
void ocall_startNetworkPartitioning();
void ocall_stopNetworkPartitioning();
void ocall_startLocalProcessing();
void ocall_stopLocalProcessing();
void ocall_storePhaseData();

/**
 * Timing for synchronization and preparations
 *

void ocall_startWindowAllocation();
void ocall_stopWindowAllocation();
void ocall_startWaitingForNetworkCompletion();
void ocall_stopWaitingForNetworkCompletion();
void ocall_startLocalProcessingPreparations();
void ocall_stopLocalProcessingPreparations();
void ocall_storeSpecialData();

/**
 * Specific counters for histogram computation
 *

void ocall_startHistogramLocalHistogramComputation();
void ocall_stopHistogramLocalHistogramComputation(uint64_t numberOfElemenets);
void ocall_startHistogramGlobalHistogramComputation();
void ocall_stopHistogramGlobalHistogramComputation();
void ocall_startHistogramAssignmentComputation();
void ocall_stopHistogramAssignmentComputation();
void ocall_startHistogramOffsetComputation();
void ocall_stopHistogramOffsetComputation();
void ocall_storeHistogramComputationData();

void ocall_startNetworkPartitioningMemoryAllocation();
void ocall_stopNetworkPartitioningMemoryAllocation(uint64_t bufferSize);
void ocall_startNetworkPartitioningMainPartitioning();
void ocall_stopNetworkPartitioningMainPartitioning(uint64_t numberOfElemenets);
void ocall_startNetworkPartitioningFlushPartitioning();
void ocall_stopNetworkPartitioningFlushPartitioning();
void ocall_startNetworkPartitioningWindowPut();
void ocall_stopNetworkPartitioningWindowPut();
void ocall_startNetworkPartitioningWindowWait();
void ocall_stopNetworkPartitioningWindowWait();
void ocall_storeNetworkPartitioningData();


/**
 * Specific ocall_counters for local partitioning
 *

void ocall_startLocalPartitioningTask();
void ocall_stopLocalPartitioningTask();
void ocall_startLocalPartitioningHistogramComputation();
void ocall_stopLocalPartitioningHistogramComputation(uint64_t numberOfElemenets);
void ocall_startLocalPartitioningOffsetComputation();
void ocall_stopLocalPartitioningOffsetComputation();
void ocall_startLocalPartitioningMemoryAllocation();
void ocall_stopLocalPartitioningMemoryAllocation(uint64_t bufferSize);
void ocall_startLocalPartitioningPartitioning();
void ocall_stopLocalPartitioningPartitioning(uint64_t numberOfElemenets);
void ocall_storeLocalPartitioningData();

/**
 * Specific ocall_counters for build-probe phase
 *

void ocall_startBuildProbeTask();
void ocall_stopBuildProbeTask();
void ocall_startBuildProbeMemoryAllocation();
void ocall_stopBuildProbeMemoryAllocation(uint64_t numberOfElemenets);
void ocall_startBuildProbeBuild();
void ocall_stopBuildProbeBuild(uint64_t numberOfElemenets);
void ocall_startBuildProbeProbe();
void ocall_stopBuildProbeProbe(uint64_t numberOfElemenets);
void ocall_storeBuildProbeData();

void ocall_writeMetaData(const char *key, char *value);
void ocall_writeMetaData_int(const char *key, uint64_t value);

void ocall_startHardwareCounters();
void ocall_printHardwareCounters(const char* name);

void ocall_printMemoryUtilization(const char* name);


void ocall_init(int32_t nodeId, uint32_t numberOfNodes, char* tag);
void ocall_serializeResults(uint64_t* result);
void ocall_sendMeasurementsToAggregator();
void ocall_receiveAllMeasurements(uint32_t numberOfNodes, uint32_t nodeId);
void ocall_printMeasurements(uint32_t numberOfNodes, uint32_t nodeId);
void ocall_storeAllMeasurements();

*/
#endif //RADIX_HASH_JOIN_MEASURE_OCALLS_H
